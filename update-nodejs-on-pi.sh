#!/bin/bash
# Update Node.js on Raspberry Pi to version 20.x
# Скрипт за обновяване на Node.js на Raspberry Pi

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Configuration
RASPBERRY_IP="192.168.4.1"
RASPBERRY_USER="vmanev"

echo -e "${YELLOW}🔄 Updating Node.js on Raspberry Pi...${NC}"
echo ""

# Connect and update Node.js
ssh "${RASPBERRY_USER}@${RASPBERRY_IP}" << 'EOF'
set -e

echo "📥 Installing Node.js 20.x..."

# Remove old Node.js if installed via apt
sudo apt remove -y nodejs npm 2>/dev/null || true

# Install Node.js 20.x using NodeSource repository
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs

# Verify installation
echo ""
echo "✅ Node.js version:"
node --version
echo "✅ npm version:"
npm --version

echo ""
echo "✨ Node.js update completed!"
EOF

echo ""
echo -e "${GREEN}✅ Node.js updated successfully on Raspberry Pi!${NC}"
