#!/bin/bash
# Update SmartCamper from Git
# Скрипт за обновяване на проекта от git и рестартиране на услугите

set -e  # Спира при грешка

# Цветове за output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}🔄 Обновяване на SmartCamper от Git...${NC}"

# Определяме пътя на проекта (от home директорията)
PROJECT_DIR="$HOME/smartCamper"

# Проверка дали директорията съществува
if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}❌ Грешка: Директорията $PROJECT_DIR не съществува!${NC}"
    exit 1
fi

# Отиваме в директорията на проекта
cd "$PROJECT_DIR"

# Проверка дали е git repository
if [ ! -d ".git" ]; then
    echo -e "${RED}❌ Грешка: $PROJECT_DIR не е git repository!${NC}"
    exit 1
fi

# Показваме текущия статус
echo -e "${YELLOW}📊 Текущ статус:${NC}"
git status --short

# Ако има локални промени само в package-lock.json, ги discard-ваме
# (този файл се генерира автоматично при npm install)
if git diff --name-only | grep -q "^frontend/package-lock.json$" && [ $(git diff --name-only | wc -l) -eq 1 ]; then
    echo -e "${YELLOW}⚠️  Отхвърляне на локални промени в package-lock.json (ще се регенерира)...${NC}"
    git checkout -- frontend/package-lock.json
fi

# Ако има други локални промени, питаме дали да продължим
if [ $(git status --porcelain | wc -l) -gt 0 ]; then
    echo -e "${YELLOW}⚠️  Има локални промени в други файлове:${NC}"
    git status --short
    if [ -t 0 ]; then
        read -p "Продължи с git pull? (локалните промени може да бъдат презаписани) (y/n) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo -e "${YELLOW}Отказано.${NC}"
            exit 0
        fi
    fi
    # Stash локалните промени
    echo -e "${YELLOW}💾 Запазване на локалните промени...${NC}"
    git stash
    STASHED=true
else
    STASHED=false
fi

# Pull от git
echo -e "${GREEN}⬇️  Изтегляне на промени от git...${NC}"
git pull

# Ако сме stash-нали промени, опитваме се да ги приложим отново
if [ "$STASHED" = true ]; then
    echo -e "${YELLOW}🔄 Опит за прилагане на запазените промени...${NC}"
    if git stash pop 2>/dev/null; then
        echo -e "${GREEN}✅ Запазените промени са приложени.${NC}"
    else
        echo -e "${YELLOW}⚠️  Има конфликти със запазените промени. Провери с 'git stash list'.${NC}"
    fi
fi

# Проверка дали има промени във frontend (сравняваме с последния commit преди pull)
FRONTEND_CHANGED=false
if git diff HEAD@{1}..HEAD --name-only 2>/dev/null | grep -q "^frontend/"; then
    FRONTEND_CHANGED=true
fi

# Проверка дали package.json е променен (трябва да инсталираме dependencies)
PACKAGE_CHANGED=false
if git diff HEAD@{1}..HEAD --name-only 2>/dev/null | grep -q "frontend/package.json\|frontend/package-lock.json"; then
    PACKAGE_CHANGED=true
fi

# Ако има промени във frontend, build-ваме
if [ "$FRONTEND_CHANGED" = true ] || [ ! -d "frontend/dist" ] || [ -z "$(ls -A frontend/dist 2>/dev/null)" ]; then
    echo -e "${YELLOW}📦 Има промени във frontend или липсва build. Build-ваме React приложението...${NC}"
    cd "$PROJECT_DIR/frontend"
    
    # Инсталираме dependencies ако няма node_modules или package.json е променен
    if [ ! -d "node_modules" ] || [ "$PACKAGE_CHANGED" = true ]; then
        echo -e "${YELLOW}📥 Инсталиране на frontend dependencies...${NC}"
        npm install
    fi
    
    # Build
    echo -e "${GREEN}🔨 Build на React приложението...${NC}"
    npm run build
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Build успешен!${NC}"
    else
        echo -e "${RED}❌ Build неуспешен!${NC}"
        exit 1
    fi
    
    cd "$PROJECT_DIR"
else
    echo -e "${GREEN}✅ Няма промени във frontend, пропускаме build.${NC}"
fi

# Проверка дали има промени в backend
BACKEND_CHANGED=false
if git diff --name-only HEAD@{1} HEAD | grep -q "^backend/"; then
    BACKEND_CHANGED=true
fi

# Рестартиране на backend service
if [ "$BACKEND_CHANGED" = true ] || [ "$FRONTEND_CHANGED" = true ]; then
    echo -e "${YELLOW}🔄 Рестартиране на backend service...${NC}"
    sudo systemctl restart smartcamper-backend
    
    # Проверка на статуса
    sleep 2
    if sudo systemctl is-active --quiet smartcamper-backend; then
        echo -e "${GREEN}✅ Backend service рестартиран успешно!${NC}"
    else
        echo -e "${RED}❌ Backend service не работи! Провери логовете:${NC}"
        echo -e "${YELLOW}   sudo journalctl -u smartcamper-backend -n 50${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}✅ Няма промени в backend, няма нужда от рестарт.${NC}"
fi

echo -e "${GREEN}✨ Обновяването завърши успешно!${NC}"
echo -e "${YELLOW}📡 Приложението е достъпно на: http://192.168.4.1:3000${NC}"
