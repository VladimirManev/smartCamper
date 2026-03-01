# Raspberry Pi - Бързи команди

## 🚀 Качване на проекта от локалния компютър (препоръчително)

Този метод работи без интернет връзка. Качва файловете директно от твоя компютър през SSH.

### От локалния компютър:

```bash
# От директорията на проекта
cd /path/to/smartCamper

# Основна команда (качва всичко)
./deploy-to-raspberry.sh

# Опции:
./deploy-to-raspberry.sh --build-frontend    # Build на frontend преди качване
./deploy-to-raspberry.sh --skip-frontend     # Пропусни frontend
./deploy-to-raspberry.sh --skip-backend      # Пропусни backend
./deploy-to-raspberry.sh --ip 192.168.4.1    # Задай IP адрес
./deploy-to-raspberry.sh --user pi           # Задай потребител
```

### Конфигурация:

Отвори `deploy-to-raspberry.sh` и промени тези стойности в началото на файла:
- `RASPBERRY_IP` - IP адрес на Raspberry Pi (обикновено `192.168.4.1`)
- `RASPBERRY_USER` - Потребителско име (обикновено `pi`)
- `SSH_KEY` - Път към SSH ключ (ако е нужно)

### Как работи:

1. Скриптът качва файловете през SSH/rsync
2. Автоматично изпълнява `update-from-local.sh` на Raspberry Pi
3. Скриптът на Pi инсталира зависимости и прави build
4. Рестартира backend service ако е нужно

## Обновяване от Git (старият метод - изисква интернет)

```bash
cd ~/smartCamper
./update-from-git.sh
```

⚠️ **Забележка:** Този метод изисква интернет връзка. За затворена система използвай метода по-горе.

## Ръчно обновяване (ако скриптът не работи)

```bash
# 1. Изтегли промени от git
cd ~/smartCamper
git pull

# 2. Build на React приложението
cd frontend
npm run build

# 3. Рестартирай backend
sudo systemctl restart smartcamper-backend
```

## Проверка на статуса

```bash
# Backend service статус
sudo systemctl status smartcamper-backend

# Логове на backend
sudo journalctl -u smartcamper-backend -n 50

# Health check
curl http://localhost:3000/health
```

## Рестартиране на услуги

```bash
# Рестартирай backend
sudo systemctl restart smartcamper-backend

# Рестартирай hostapd (WiFi Access Point)
sudo systemctl restart hostapd
```

## Полезни команди

```bash
# Проверка на портовете
sudo netstat -tlnp | grep -E '1883|3000'

# Проверка на свързани устройства към WiFi
iw dev wlan0 station dump

# Проверка на IP адрес
hostname -I
```

## Доступ до приложението

- **HTTP:** http://192.168.4.1:3000
- **Health Check:** http://192.168.4.1:3000/health

## Правилно изключване на Raspberry Pi

⚠️ **ВАЖНО:** Никога не изключвай Raspberry Pi директно! Това може да повреди файловата система.

### Безопасно изключване:

```bash
# Опция 1: Използвай shutdown командата
sudo shutdown -h now

# Опция 2: Използвай halt командата
sudo halt

# Опция 3: Използвай poweroff командата
sudo poweroff
```

### Изчакване преди физическо изключване:

След изпълнение на командата, изчакай докато:

- LED индикаторът спре да мига (обикновено 10-30 секунди)
- Няма активност на SD картата
- След това можеш безопасно да изключиш захранването

### Проверка дали е безопасно:

```bash
# Проверка дали системата е спряла
# Ако командата не отговаря, системата е спряла
sudo systemctl status
```

### Рестартиране (без изключване):

```bash
# Рестартирай Raspberry Pi
sudo reboot
```
