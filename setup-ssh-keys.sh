#!/bin/bash
# Setup SSH keys for passwordless access to Raspberry Pi
# Скрипт за настройване на SSH ключове за безпаролен достъп

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
RASPBERRY_IP="192.168.4.1"
RASPBERRY_USER="vmanev"
SSH_KEY_PATH="$HOME/.ssh/id_rsa"

echo -e "${BLUE}🔐 Setting up SSH keys for passwordless access...${NC}"
echo ""

# Check if SSH key already exists
if [ -f "$SSH_KEY_PATH" ]; then
    echo -e "${YELLOW}⚠️  SSH key already exists at: $SSH_KEY_PATH${NC}"
    read -p "Do you want to use existing key? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${YELLOW}Generating new SSH key...${NC}"
        ssh-keygen -t rsa -b 4096 -f "$SSH_KEY_PATH" -N ""
    else
        echo -e "${GREEN}✅ Using existing SSH key${NC}"
    fi
else
    echo -e "${YELLOW}📝 Generating new SSH key...${NC}"
    echo -e "${YELLOW}   Press Enter to accept default location: $SSH_KEY_PATH${NC}"
    echo -e "${YELLOW}   You can set a passphrase or leave it empty for passwordless access${NC}"
    ssh-keygen -t rsa -b 4096 -f "$SSH_KEY_PATH"
fi

echo ""
echo -e "${YELLOW}📤 Copying SSH key to Raspberry Pi...${NC}"
echo -e "${YELLOW}   You will be asked for your password one last time${NC}"
echo ""

# Copy SSH key to Raspberry Pi
ssh-copy-id -i "$SSH_KEY_PATH.pub" "${RASPBERRY_USER}@${RASPBERRY_IP}"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✅ SSH key copied successfully!${NC}"
    echo ""
    
    # Test passwordless connection
    echo -e "${YELLOW}🔌 Testing passwordless connection...${NC}"
    if ssh -o BatchMode=yes -o ConnectTimeout=5 -i "$SSH_KEY_PATH" "${RASPBERRY_USER}@${RASPBERRY_IP}" "echo 'Connection successful'" &> /dev/null; then
        echo -e "${GREEN}✅ Passwordless SSH connection works!${NC}"
        echo ""
        echo -e "${GREEN}✨ Setup completed successfully!${NC}"
        echo -e "${YELLOW}   You can now use deploy-to-raspberry.sh without entering password!${NC}"
    else
        echo -e "${YELLOW}⚠️  Passwordless connection test failed.${NC}"
        echo -e "${YELLOW}   You may need to check SSH configuration on Raspberry Pi.${NC}"
    fi
else
    echo ""
    echo -e "${RED}❌ Failed to copy SSH key!${NC}"
    echo -e "${YELLOW}   Make sure:${NC}"
    echo -e "${YELLOW}   - You are connected to the WiFi network${NC}"
    echo -e "${YELLOW}   - Raspberry Pi is accessible${NC}"
    echo -e "${YELLOW}   - SSH is enabled on Raspberry Pi${NC}"
    exit 1
fi

echo ""
echo -e "${BLUE}📝 Next steps:${NC}"
echo -e "${YELLOW}   1. Add SSH key to ssh-agent (to avoid entering passphrase every time):${NC}"
echo -e "${YELLOW}      ssh-add $SSH_KEY_PATH${NC}"
echo -e "${YELLOW}   2. Or generate a new key without passphrase for passwordless access${NC}"
echo -e "${YELLOW}   3. deploy-to-raspberry.sh will automatically detect and use the key${NC}"
