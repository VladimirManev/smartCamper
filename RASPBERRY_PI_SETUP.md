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

## 📋 Стъпка 9: Автоматично изчистване на стари WiFi свързания

**Проблем:** ESP32 не може да се свърже след рестарт, защото hostapd не освобождава автоматично старите station записи.

**Решение:** Скрипт който периодично изчиства неактивни свързания чрез ping проверка.

```bash
# Създай скрипта
sudo nano /usr/local/bin/cleanup-wifi-stations.sh
```

Постави следното:

```bash
#!/bin/bash
# Cleanup inactive WiFi stations script
# Removes stations that don't respond to ping (not actually connected)

# Check each connected device
iw dev wlan0 station dump 2>/dev/null | grep "Station" | awk '{print $2}' | while read MAC; do
    if [ ! -z "$MAC" ]; then
        # Get device information
        STATION_INFO=$(iw dev wlan0 station get "$MAC" 2>/dev/null)

        # Check if device has IP address in DHCP leases
        DHCP_LEASE=$(cat /var/lib/misc/dnsmasq.leases 2>/dev/null | grep -i "$MAC")

        if [ -z "$DHCP_LEASE" ]; then
            # No DHCP lease - definitely not connected, clean it up
            echo "$(date '+%H:%M:%S'): Cleaning up station without DHCP lease: $MAC"
            sudo iw dev wlan0 station del "$MAC" 2>/dev/null
        else
            # Extract IP address from DHCP lease (format: timestamp mac ip hostname)
            IP_ADDRESS=$(echo "$DHCP_LEASE" | awk '{print $3}')

            if [ ! -z "$IP_ADDRESS" ]; then
                # Ping the device (1 ping, 1 second timeout)
                if ping -c 1 -W 1 "$IP_ADDRESS" > /dev/null 2>&1; then
                    # Device responds to ping - keep it
                    echo "$(date '+%H:%M:%S'): Keeping station that responds to ping: $MAC ($IP_ADDRESS)"
                else
                    # Device doesn't respond to ping - clean it up
                    echo "$(date '+%H:%M:%S'): Cleaning up station that doesn't respond to ping: $MAC ($IP_ADDRESS)"
                    sudo iw dev wlan0 station del "$MAC" 2>/dev/null
                fi
            else
                # No IP address in lease - clean it up
                echo "$(date '+%H:%M:%S'): Cleaning up station with invalid lease: $MAC"
                sudo iw dev wlan0 station del "$MAC" 2>/dev/null
            fi
        fi
    fi
done
```

```bash
# Направи го изпълним
sudo chmod +x /usr/local/bin/cleanup-wifi-stations.sh

# Създай systemd service
sudo nano /etc/systemd/system/cleanup-wifi-stations.service
```

Постави:

```
[Unit]
Description=Cleanup inactive WiFi stations

[Service]
Type=oneshot
ExecStart=/usr/local/bin/cleanup-wifi-stations.sh
```

```bash
# Създай systemd timer (на всеки 30 секунди)
sudo nano /etc/systemd/system/cleanup-wifi-stations.timer
```

Постави:

```
[Unit]
Description=Cleanup WiFi stations timer

[Timer]
OnBootSec=30s
OnUnitActiveSec=30s

[Install]
WantedBy=timers.target
```

```bash
# Активирай timer
sudo systemctl daemon-reload
sudo systemctl enable cleanup-wifi-stations.timer
sudo systemctl start cleanup-wifi-stations.timer

# Проверка
sudo systemctl status cleanup-wifi-stations.timer
```

**Важно:** Скриптът изчиства само устройства които:

- Нямат DHCP lease ИЛИ
- Не отговарят на ping

Ако устройството е активно свързано и комуникира, ще отговаря на ping и няма да се изчисти.

**Потенциален проблем:** Устройства които не отговарят на ping по дизайн (firewall/security) могат да се изчистват по грешка. В този случай те ще се свържат отново автоматично.

## 📋 Стъпка 10: Стартиране на backend сървъра като услуга

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

## 📋 Стъпка 11: Проверка

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

---

## ⚠️ Важни бележки

### WiFi Cleanup Script - Потенциални проблеми

**Как работи:**

- Скриптът ping-ва всички свързани устройства на всеки 30 секунди
- Ако устройството не отговаря на ping → изчиства station записа
- Това позволява на ESP32 да се свърже отново след рестарт

**Потенциални проблеми:**

1. **Устройства които не отговарят на ping:**

   - Някои устройства може да имат firewall който блокира ping
   - В този случай ще се изчистват по грешка, но ще се свържат отново
   - За ESP32 това не е проблем (отговаря на ping)

2. **Много устройства:**

   - Ако има много устройства, ping-ването може да отнеме време
   - Използваме 1 секунда timeout, така че е бързо
   - При 10 устройства = максимум 10 секунди

3. **False positives:**
   - Временни мрежови проблеми могат да причинят неуспешен ping
   - Но следващия път (след 30 секунди) ще се провери отново
   - Ако устройството е активно, ще отговори на следващия ping

**Заключение:** Скриптът е безопасен за нормална употреба. ESP32 и повечето IoT устройства отговарят на ping и няма проблеми.

---

**Последно обновяване:** 2025-11-26  
**Включва:**

- Systemd service настройка с troubleshooting за CHDIR грешки
- WiFi cleanup script за автоматично изчистване на стари свързания
