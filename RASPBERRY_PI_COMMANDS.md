# Raspberry Pi - Бързи команди

## Обновяване от Git

```bash
cd ~/smartCamper
./update-from-git.sh
```

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
