#!/bin/bash
# Clean node_modules on Raspberry Pi
# Скрипт за премахване на node_modules на Raspberry Pi

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Configuration
RASPBERRY_IP="192.168.4.1"
RASPBERRY_USER="vmanev"

echo -e "${YELLOW}🧹 Cleaning node_modules on Raspberry Pi...${NC}"
echo ""

# Connect and clean
ssh "${RASPBERRY_USER}@${RASPBERRY_IP}" << 'EOF'
cd ~/smartCamper

# Remove frontend node_modules
if [ -d "frontend/node_modules" ]; then
    echo "Removing frontend/node_modules..."
    rm -rf frontend/node_modules
    echo "✅ Frontend node_modules removed"
fi

# Remove backend node_modules
if [ -d "backend/node_modules" ]; then
    echo "Removing backend/node_modules..."
    rm -rf backend/node_modules
    echo "✅ Backend node_modules removed"
fi

# Remove package-lock.json files
if [ -f "frontend/package-lock.json" ]; then
    rm -f frontend/package-lock.json
    echo "✅ Frontend package-lock.json removed"
fi

if [ -f "backend/package-lock.json" ]; then
    rm -f backend/package-lock.json
    echo "✅ Backend package-lock.json removed"
fi

echo ""
echo "✨ Cleanup completed!"
EOF

echo ""
echo -e "${GREEN}✅ Cleanup completed on Raspberry Pi!${NC}"
