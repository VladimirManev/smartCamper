# Raspberry Pi Setup - SmartCamper

Стегната инструкция за настройка на нов Raspberry Pi от нулата.

## 📋 Стъпка 1: Инсталация на операционна система

1. Изтегли Raspberry Pi Imager от: https://www.raspberrypi.com/software/
2. Избери Raspberry Pi OS (64-bit) Lite или Desktop
3. Запиши на SD карта
4. Постави SD картата в Pi и стартирай

## 📋 Стъпка 2: Първоначална конфигурация

```bash
# Промени паролата (ако не е направено при първо стартиране)
passwd

# Обнови системата
sudo apt update && sudo apt upgrade -y

# Инсталирай git (ако няма)
sudo apt install git -y
```

## 📋 Стъпка 3: Инсталация на Node.js

```bash
# Инсталирай Node.js 20.x (LTS)
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs

# Проверка
node --version
npm --version
```

## 📋 Стъпка 4: Клониране на проекта

```bash
# Отиди в home директорията
cd ~

# Клонирай проекта (замени с твоя GitHub URL)
git clone https://github.com/tvoi-username/smartCamper.git

# Или ако нямаш GitHub, създай директорията ръчно
mkdir -p ~/smartCamper
# След това качи файловете чрез SCP или друг метод
```

## 📋 Стъпка 5: Инсталация на backend зависимости

```bash
cd ~/smartCamper/backend
npm install
```

## 📋 Стъпка 6: Конфигуриране на hostapd (WiFi Access Point)

```bash
# Инсталирай hostapd
sudo apt install hostapd -y

# Създай конфигурационен файл
sudo nano /etc/hostapd/hostapd.conf
```

Постави следното съдържание:

```
interface=wlan0
driver=nl80211
ssid=SmartCamper
channel=6
hw_mode=g
wmm_enabled=0
macaddr_acl=0
auth_algs=1
wpa=2
wpa_passphrase=12344321
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP CCMP
rsn_pairwise=CCMP
ignore_broadcast_ssid=0
```

```bash
# Редактирай /etc/default/hostapd
sudo nano /etc/default/hostapd
```

Намери реда `#DAEMON_CONF=""` и промени на:

```
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

```bash
# Стартирай и включи hostapd
sudo systemctl start hostapd
sudo systemctl enable hostapd

# Проверка
sudo systemctl status hostapd
```

## 📋 Стъпка 7: Конфигуриране на DHCP за wlan0

```bash
# Редактирай dhcpcd.conf
sudo nano /etc/dhcpcd.conf
```

Добави в края на файла:

```
interface wlan0
static ip_address=192.168.4.1/24
nohook wpa_supplicant
```

```bash
# Рестартирай dhcpcd
sudo systemctl restart dhcpcd

# Проверка на IP адреса
hostname -I
```

## 📋 Стъпка 8: Конфигуриране на DHCP сървър (dnsmasq)

```bash
# Инсталирай dnsmasq
sudo apt install dnsmasq -y

# Backup на оригиналната конфигурация
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig

# Създай нова конфигурация
sudo nano /etc/dnsmasq.conf
```

Постави следното:

```
interface=wlan0
dhcp-range=192.168.4.2,192.168.4.20,255.255.255.0,24h
```

```bash
# Рестартирай dnsmasq
sudo systemctl restart dnsmasq
sudo systemctl enable dnsmasq
```

## 📋 Стъпка 9: Стартиране на backend сървъра като услуга

```bash
# Първо определи твоя потребител и път
whoami
pwd

# Създай systemd service файл
sudo nano /etc/systemd/system/smartcamper-backend.service
```

**ВАЖНО:** Замени `vmanev` с твоя потребител и `/home/vmanev` с твоя home директория!

Постави следното:

```
[Unit]
Description=SmartCamper Backend Server
After=network.target

[Service]
Type=simple
User=vmanev
WorkingDirectory=/home/vmanev/smartCamper/backend
ExecStart=/usr/bin/node /home/vmanev/smartCamper/backend/server.js
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

```bash
# Рестартирай systemd
sudo systemctl daemon-reload

# Стартирай услугата
sudo systemctl start smartcamper-backend

# Включи автоматично стартиране при boot
sudo systemctl enable smartcamper-backend

# Проверка на статуса
sudo systemctl status smartcamper-backend
```

**Ако видиш грешка `status=200/CHDIR`:**

- Проверка на потребителя: `whoami`
- Проверка на пътя: `ls -la ~/smartCamper/backend/server.js`
- Обнови `User=` и `WorkingDirectory=` в service файла с правилните стойности

## 📋 Стъпка 10: Проверка

```bash
# Проверка на hostapd
sudo systemctl status hostapd

# Проверка на backend
sudo systemctl status smartcamper-backend

# Проверка на портовете
sudo netstat -tlnp | grep -E '1883|3000'

# Тест на health endpoint
curl http://localhost:3000/health
```

## 🔧 Полезни команди за поддръжка

```bash
# Виж логовете на backend
sudo journalctl -u smartcamper-backend -f

# Рестартирай backend
sudo systemctl restart smartcamper-backend

# Рестартирай hostapd
sudo systemctl restart hostapd

# Проверка на свързани устройства към WiFi
iw dev wlan0 station dump
```

## 🚨 При проблеми

### Backend не стартира

```bash
# Проверка на логовете
sudo journalctl -u smartcamper-backend -n 50

# Проверка дали Node.js работи
node --version

# Проверка дали зависимостите са инсталирани
cd ~/smartCamper/backend && npm install
```

### Грешка `status=200/CHDIR` при стартиране

Това означава, че systemd не може да промени директорията. Проверка:

```bash
# Определи правилния потребител
whoami

# Определи правилния път
pwd
ls -la ~/smartCamper/backend/server.js

# Обнови service файла с правилните стойности
sudo nano /etc/systemd/system/smartcamper-backend.service
# Промени User= и WorkingDirectory= с правилните стойности

# Рестартирай
sudo systemctl daemon-reload
sudo systemctl restart smartcamper-backend
```

### WiFi Access Point не работи

```bash
# Проверка на hostapd логовете
sudo journalctl -u hostapd -n 50

# Проверка на wlan0 интерфейса
iwconfig wlan0

# Рестартирай networking
sudo systemctl restart networking
```

### IP адрес не е 192.168.4.1

```bash
# Проверка на dhcpcd конфигурацията
cat /etc/dhcpcd.conf | grep wlan0

# Рестартирай dhcpcd
sudo systemctl restart dhcpcd
```

---

**Последно обновяване:** 2025-11-13  
**Включва:** Systemd service настройка с troubleshooting за CHDIR грешки
