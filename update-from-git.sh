#!/bin/bash
# Update SmartCamper from Git
# Script for updating project from git and restarting services

set -e  # Stop on error

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}🔄 Updating SmartCamper from Git...${NC}"

# Determine project path (from home directory)
PROJECT_DIR="$HOME/smartCamper"

# Check if directory exists
if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}❌ Error: Directory $PROJECT_DIR does not exist!${NC}"
    exit 1
fi

# Go to project directory
cd "$PROJECT_DIR"

# Check if it's a git repository
if [ ! -d ".git" ]; then
    echo -e "${RED}❌ Error: $PROJECT_DIR is not a git repository!${NC}"
    exit 1
fi

# Show current status
echo -e "${YELLOW}📊 Current status:${NC}"
git status --short

# If there are local changes only in package-lock.json, discard them
# (this file is auto-generated on npm install)
if git diff --name-only | grep -q "^frontend/package-lock.json$" && [ $(git diff --name-only | wc -l) -eq 1 ]; then
    echo -e "${YELLOW}⚠️  Discarding local changes in package-lock.json (will be regenerated)...${NC}"
    git checkout -- frontend/package-lock.json
fi

# If there are other local changes, ask if we should continue
if [ $(git status --porcelain | wc -l) -gt 0 ]; then
    echo -e "${YELLOW}⚠️  There are local changes in other files:${NC}"
    git status --short
    if [ -t 0 ]; then
        read -p "Continue with git pull? (local changes may be overwritten) (y/n) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            echo -e "${YELLOW}Cancelled.${NC}"
            exit 0
        fi
    fi
    # Stash local changes
    echo -e "${YELLOW}💾 Saving local changes...${NC}"
    git stash
    STASHED=true
else
    STASHED=false
fi

# Pull from git
echo -e "${GREEN}⬇️  Pulling changes from git...${NC}"
git pull

# If we stashed changes, try to apply them again
if [ "$STASHED" = true ]; then
    echo -e "${YELLOW}🔄 Attempting to apply stashed changes...${NC}"
    if git stash pop 2>/dev/null; then
        echo -e "${GREEN}✅ Stashed changes applied.${NC}"
    else
        echo -e "${YELLOW}⚠️  There are conflicts with stashed changes. Check with 'git stash list'.${NC}"
    fi
fi

# Check if there are changes in frontend (compare with last commit before pull)
FRONTEND_CHANGED=false
if git diff HEAD@{1}..HEAD --name-only 2>/dev/null | grep -q "^frontend/"; then
    FRONTEND_CHANGED=true
fi

# Check if package.json changed (need to install dependencies)
PACKAGE_CHANGED=false
if git diff HEAD@{1}..HEAD --name-only 2>/dev/null | grep -q "frontend/package.json\|frontend/package-lock.json"; then
    PACKAGE_CHANGED=true
fi

# If there are changes in frontend, build
if [ "$FRONTEND_CHANGED" = true ] || [ ! -d "frontend/dist" ] || [ -z "$(ls -A frontend/dist 2>/dev/null)" ]; then
    echo -e "${YELLOW}📦 There are changes in frontend or build is missing. Building React application...${NC}"
    cd "$PROJECT_DIR/frontend"
    
    # Install dependencies if node_modules doesn't exist or package.json changed
    if [ ! -d "node_modules" ] || [ "$PACKAGE_CHANGED" = true ]; then
        echo -e "${YELLOW}📥 Installing frontend dependencies...${NC}"
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
    echo -e "${GREEN}✅ No changes in frontend, skipping build.${NC}"
fi

# Check if there are changes in backend
BACKEND_CHANGED=false
if git diff --name-only HEAD@{1} HEAD | grep -q "^backend/"; then
    BACKEND_CHANGED=true
fi

# Restart backend service
if [ "$BACKEND_CHANGED" = true ] || [ "$FRONTEND_CHANGED" = true ]; then
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
    echo -e "${GREEN}✅ No changes in backend, no restart needed.${NC}"
fi

echo -e "${GREEN}✨ Update completed successfully!${NC}"
echo -e "${YELLOW}📡 Application is available at: http://192.168.4.1:3000${NC}"
