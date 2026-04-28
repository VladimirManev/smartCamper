# Инструкции за Deployment - SmartCamper

## 🚀 Бърз старт

### Стъпка 1: Подготовка
1. Увери се, че си свързан към WiFi мрежата на Raspberry Pi (обикновено "SmartCamper")
2. Провери дали Raspberry Pi е включен: `ping 192.168.4.1`
3. Провери SSH достъпа: `ssh vmanev@192.168.4.1`

### Стъпка 2: Качване на проекта
От директорията на проекта изпълни:

```bash
./deploy-to-raspberry.sh
```

За Raspberry Pi без интернет (препоръчително):

```bash
./deploy-to-raspberry.sh --offline-pi
```

Скриптът автоматично:
- ✅ Тества SSH връзката
- ✅ Качва променените файлове (backend, frontend)
- ✅ Инсталира зависимости (ако е нужно)
- ✅ Прави build на frontend (стандартен режим)
- ✅ Рестартира backend service

## 📋 Опции

```bash
# Offline режим за Pi без интернет (build локално, качва dist)
./deploy-to-raspberry.sh --offline-pi

# Build на frontend преди качване (по-бързо на Pi)
./deploy-to-raspberry.sh --build-frontend

# Пропусни frontend (само backend)
./deploy-to-raspberry.sh --skip-frontend

# Пропусни backend (само frontend)
./deploy-to-raspberry.sh --skip-backend

# Задай SSH парола
./deploy-to-raspberry.sh --password твоята_парола

# Помощ
./deploy-to-raspberry.sh --help
```

## 🔧 Конфигурация

Отвори `deploy-to-raspberry.sh` и промени в началото на файла:

```bash
RASPBERRY_IP="192.168.4.1"    # IP адрес на Raspberry Pi
RASPBERRY_USER="vmanev"       # Потребителско име
RASPBERRY_PATH="~/smartCamper" # Път към проекта
```

## 🔐 Безпаролен достъп (препоръчително)

### Най-лесният начин: Автоматично настройване на SSH ключове
```bash
./setup-ssh-keys.sh
```

Скриптът автоматично:
- ✅ Генерира SSH ключ (ако нямаш)
- ✅ Копира ключа на Raspberry Pi
- ✅ Тества безпаролния достъп

**Важно:** Ако зададеш passphrase (парола) на SSH ключа:
- При първо изпълнение на `deploy-to-raspberry.sh` ще те попита за passphrase веднъж
- Скриптът автоматично добавя ключа към `ssh-agent`, който кешира passphrase-а
- След това няма да пита за passphrase при всяка команда

**За напълно безпаролен достъп:**
- При генериране на ключа, натисни Enter за празен passphrase
- Или ръчно добави ключа към ssh-agent: `ssh-add ~/.ssh/id_rsa`

### Алтернатива 1: Ръчно настройване на SSH ключове
```bash
# Генерирай SSH ключ (ако нямаш)
ssh-keygen -t rsa -b 4096

# Копирай ключа на Raspberry Pi
ssh-copy-id vmanev@192.168.4.1

# Добави ключа към ssh-agent (за да не пита за passphrase)
ssh-add ~/.ssh/id_rsa
```

### Алтернатива 2: Използвай sshpass
```bash
# Инсталирай на macOS
brew install hudochenkov/sshpass/sshpass

# След това използвай с --password опция
./deploy-to-raspberry.sh --password твоята_парола
```

### Защо ssh-agent?

Ако SSH ключът ти има passphrase, `ssh-agent` кешира паролата за текущата сесия:
- Въвеждаш passphrase веднъж при стартиране
- След това всички SSH команди работят без да питат за парола
- Скриптът `deploy-to-raspberry.sh` автоматично използва `ssh-agent`

## 🛠️ Отстраняване на проблеми

### Проблем: Не може да се свърже през SSH
**Решение:**
1. Провери дали си свързан към правилната WiFi мрежа
2. Провери IP адреса: `ping 192.168.4.1`
3. Провери дали SSH е активиран на Pi: `sudo systemctl status ssh`

### Проблем: npm install грешки (ENOTEMPTY)
**Решение:**
```bash
# Почисти node_modules на Pi
./clean-node-modules-on-pi.sh

# Или ръчно:
ssh vmanev@192.168.4.1
cd ~/smartCamper
rm -rf frontend/node_modules backend/node_modules
```

### Проблем: Node.js версия е стара
**Решение:**
```bash
# Обнови Node.js на Pi
./update-nodejs-on-pi.sh

# Или ръчно:
ssh vmanev@192.168.4.1
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs
```

### Проблем: Build се проваля
**Решение:**
1. Провери Node.js версията: `node --version` (трябва да е 20.x или 22.x)
2. Провери логове: `sudo journalctl -u smartcamper-backend -n 50`
3. Ако Pi няма интернет, използвай `./deploy-to-raspberry.sh --offline-pi`
4. Опитай ръчно build (когато има интернет): 
   ```bash
   ssh vmanev@192.168.4.1
   cd ~/smartCamper/frontend
   rm -rf node_modules
   npm install
   npm run build
   ```

## 📝 Често срещани команди

```bash
# Качи промените
./deploy-to-raspberry.sh

# Качи офлайн (Pi без интернет)
./deploy-to-raspberry.sh --offline-pi

# Почисти node_modules на Pi
./clean-node-modules-on-pi.sh

# Обнови Node.js на Pi
./update-nodejs-on-pi.sh

# Провери статуса на backend на Pi
ssh vmanev@192.168.4.1 "sudo systemctl status smartcamper-backend"

# Виж логове на backend
ssh vmanev@192.168.4.1 "sudo journalctl -u smartcamper-backend -n 50"
```

## ✅ След успешен deployment

Приложението е достъпно на: **http://192.168.4.1:3000**

Можеш да отвориш в браузъра и да видиш промените.

## 📌 Бележки

- Скриптът качва само променените файлове (rsync)
- `node_modules` не се качват
- `dist` се качва само при `--offline-pi`
- Backend service се рестартира автоматично след промени
- Ако има проблеми, виж секцията "Отстраняване на проблеми" по-горе
