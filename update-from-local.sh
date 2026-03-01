#!/bin/bash
# Update SmartCamper from Local Files
# Скрипт за обновяване на проекта от локални файлове (качени през SSH)
# Този скрипт се изпълнява на Raspberry Pi след качване на файловете

set -e  # Stop on error

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}🔄 Updating SmartCamper from local files...${NC}"

# Determine project path (from home directory)
PROJECT_DIR="$HOME/smartCamper"

# Check if directory exists
if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}❌ Error: Directory $PROJECT_DIR does not exist!${NC}"
    exit 1
fi

# Go to project directory
cd "$PROJECT_DIR"

# Check if frontend needs to be built
FRONTEND_NEEDS_BUILD=false
if [ -d "frontend" ]; then
    # Build if dist doesn't exist or package.json is newer than dist
    if [ ! -d "frontend/dist" ] || [ -z "$(ls -A frontend/dist 2>/dev/null)" ] || \
       ([ -f "frontend/package.json" ] && [ "frontend/package.json" -nt "frontend/dist" ]); then
        FRONTEND_NEEDS_BUILD=true
    fi
fi

# Build frontend if needed
if [ "$FRONTEND_NEEDS_BUILD" = true ]; then
    echo -e "${YELLOW}📦 Building React application...${NC}"
    cd "$PROJECT_DIR/frontend"
    
    # Install dependencies if needed
    if [ ! -d "node_modules" ] || \
       ([ -f "package.json" ] && [ "package.json" -nt "node_modules" ]); then
        echo -e "${YELLOW}📥 Installing frontend dependencies...${NC}"
        # Remove node_modules if it exists to avoid ENOTEMPTY errors
        if [ -d "node_modules" ]; then
            echo -e "${YELLOW}🧹 Cleaning old node_modules...${NC}"
            rm -rf node_modules
        fi
        # Also remove package-lock.json to ensure clean install
        if [ -f "package-lock.json" ]; then
            rm -f package-lock.json
        fi
        npm install
    fi
    
    # Build
    echo -e "${GREEN}🔨 Building React application...${NC}"
    npm run build
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Build successful!${NC}"
    else
        echo -e "${RED}❌ Build failed!${NC}"
        exit 1
    fi
    
    cd "$PROJECT_DIR"
else
    echo -e "${GREEN}✅ Frontend build is up to date.${NC}"
fi

# Install backend dependencies if needed
BACKEND_NEEDS_RESTART=false
if [ -d "backend" ]; then
    if [ ! -d "backend/node_modules" ] || \
       ([ -f "backend/package.json" ] && [ "backend/package.json" -nt "backend/node_modules" ]); then
        echo -e "${YELLOW}📥 Installing backend dependencies...${NC}"
        cd "$PROJECT_DIR/backend"
        # Remove node_modules if it exists to avoid ENOTEMPTY errors
        if [ -d "node_modules" ]; then
            echo -e "${YELLOW}🧹 Cleaning old node_modules...${NC}"
            rm -rf node_modules
        fi
        npm install
        cd "$PROJECT_DIR"
        BACKEND_NEEDS_RESTART=true
    fi
    
    # Check if any backend files were updated (compare with node_modules as reference)
    if [ -d "backend/node_modules" ]; then
        # If any .js file in backend is newer than node_modules, restart needed
        if find backend -type f -name "*.js" -newer backend/node_modules 2>/dev/null | grep -q .; then
            BACKEND_NEEDS_RESTART=true
        fi
    fi
fi

# Restart backend service if needed
if [ "$BACKEND_NEEDS_RESTART" = true ] || [ "$FRONTEND_NEEDS_BUILD" = true ]; then
    echo -e "${YELLOW}🔄 Restarting backend service...${NC}"
    sudo systemctl restart smartcamper-backend
    
    # Check status
    sleep 2
    if sudo systemctl is-active --quiet smartcamper-backend; then
        echo -e "${GREEN}✅ Backend service restarted successfully!${NC}"
    else
        echo -e "${RED}❌ Backend service is not running! Check logs:${NC}"
        echo -e "${YELLOW}   sudo journalctl -u smartcamper-backend -n 50${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}✅ No restart needed.${NC}"
fi

echo -e "${GREEN}✨ Update completed successfully!${NC}"
echo -e "${YELLOW}📡 Application is available at: http://192.168.4.1:3000${NC}"
