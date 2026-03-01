# Deployment Guide - SmartCamper

Ръководство за качване на проекта на Raspberry Pi.

## 📚 Инструкции

- **🇧🇬 Български:** [DEPLOY_INSTRUCTIONS_BG.md](./DEPLOY_INSTRUCTIONS_BG.md)
- **🇬🇧 English:** [DEPLOY_INSTRUCTIONS_EN.md](./DEPLOY_INSTRUCTIONS_EN.md)

За бърз старт виж един от файловете по-горе.

## 🚀 Качване през SSH (Препоръчително за затворена система)

Този метод работи без интернет връзка. Качва файловете директно от твоя компютър през SSH.

### Първоначална настройка

1. **Отвори `deploy-to-raspberry.sh`** и промени конфигурацията в началото на файла:

```bash
RASPBERRY_IP="192.168.4.1"    # IP адрес на Raspberry Pi
RASPBERRY_USER="pi"            # Потребителско име
RASPBERRY_PATH="~/smartCamper" # Път към проекта
SSH_KEY=""                     # Път към SSH ключ (ако е нужно)
```

2. **Увери се, че си свързан към WiFi мрежата на Raspberry Pi** (обикновено "SmartCamper")

3. **Провери SSH достъпа:**
```bash
ssh pi@192.168.4.1
```

### Качване на проекта

От директорията на проекта на локалния компютър:

```bash
./deploy-to-raspberry.sh
```

### Опции

```bash
# Build на frontend преди качване (по-бързо на Pi)
./deploy-to-raspberry.sh --build-frontend

# Пропусни frontend (само backend)
./deploy-to-raspberry.sh --skip-frontend

# Пропусни backend (само frontend)
./deploy-to-raspberry.sh --skip-backend

# Задай IP адрес
./deploy-to-raspberry.sh --ip 192.168.4.1

# Задай потребител
./deploy-to-raspberry.sh --user pi

# Помощ
./deploy-to-raspberry.sh --help
```

### Как работи

1. Скриптът тества SSH връзката
2. Качва файловете през `rsync` (само променените файлове)
3. Автоматично изпълнява `update-from-local.sh` на Raspberry Pi
4. Скриптът на Pi:
   - Инсталира зависимости ако е нужно
   - Прави build на frontend
   - Рестартира backend service

### Какво се качва

- ✅ Всички source файлове (backend/, frontend/src/)
- ✅ Конфигурационни файлове
- ✅ package.json файлове
- ❌ node_modules/ (инсталират се на Pi)
- ❌ frontend/dist/ (build се на Pi)
- ❌ .git/ директория
- ❌ Временни и IDE файлове

## 📋 Старият метод (Git - изисква интернет)

Ако Raspberry Pi има достъп до интернет, можеш да използваш стария метод:

```bash
# На Raspberry Pi
cd ~/smartCamper
./update-from-git.sh
```

⚠️ **Забележка:** Този метод изисква интернет връзка и Git repository.

## 🔧 Отстраняване на проблеми

### Не може да се свърже през SSH

1. Провери дали си свързан към правилната WiFi мрежа
2. Провери IP адреса: `ping 192.168.4.1`
3. Провери дали SSH е активиран на Pi: `sudo systemctl status ssh`

### rsync не е инсталиран

**На macOS:**
```bash
brew install rsync
```

**На Linux:**
```bash
sudo apt-get install rsync
```

### Build се проваля на Pi

1. Провери дали Node.js е инсталиран: `node --version`
2. Провери логове: `sudo journalctl -u smartcamper-backend -n 50`
3. Опитай ръчно build: `cd ~/smartCamper/frontend && npm install && npm run build`

### Backend не рестартира

1. Провери статуса: `sudo systemctl status smartcamper-backend`
2. Провери логове: `sudo journalctl -u smartcamper-backend -n 50`
3. Рестартирай ръчно: `sudo systemctl restart smartcamper-backend`

## 📝 Бележки

- Скриптът използва `rsync` за ефективно качване (само променените файлове)
- Frontend build се прави на Raspberry Pi (по-добре за производителност)
- Backend service се рестартира автоматично след качване
- Скриптът пропуска `node_modules` и `dist` за по-бързо качване
