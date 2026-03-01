#!/bin/bash
# Install Shutdown Button Service on Raspberry Pi

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Installing Shutdown Button Service...${NC}"
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$SCRIPT_DIR"

# Check if files exist
if [ ! -f "$PROJECT_DIR/shutdown-button.py" ]; then
    echo -e "${RED}Error: shutdown-button.py not found!${NC}"
    exit 1
fi

if [ ! -f "$PROJECT_DIR/shutdown-button.service" ]; then
    echo -e "${RED}Error: shutdown-button.service not found!${NC}"
    exit 1
fi

# Check Python 3
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}Error: Python 3 not installed!${NC}"
    exit 1
fi

# Install RPi.GPIO if needed
echo -e "${YELLOW}Checking RPi.GPIO...${NC}"
if ! python3 -c "import RPi.GPIO" 2>/dev/null; then
    echo -e "${YELLOW}Installing RPi.GPIO...${NC}"
    sudo pip3 install RPi.GPIO
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}RPi.GPIO installed successfully${NC}"
    else
        echo -e "${RED}Failed to install RPi.GPIO${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}RPi.GPIO already installed${NC}"
fi

# Make script executable
echo -e "${YELLOW}Setting permissions...${NC}"
chmod +x "$PROJECT_DIR/shutdown-button.py"
echo -e "${GREEN}Permissions set${NC}"

# Copy service file
echo -e "${YELLOW}Copying service file...${NC}"
sudo cp "$PROJECT_DIR/shutdown-button.service" /etc/systemd/system/

# Update service file with correct path
echo -e "${YELLOW}Updating paths in service file...${NC}"
sudo sed -i "s|/home/vmanev/smartCamper|$PROJECT_DIR|g" /etc/systemd/system/shutdown-button.service
echo -e "${GREEN}Paths updated${NC}"

# Reload systemd
echo -e "${YELLOW}Reloading systemd...${NC}"
sudo systemctl daemon-reload
echo -e "${GREEN}Systemd reloaded${NC}"

# Enable service
echo -e "${YELLOW}Enabling service (auto-start on boot)...${NC}"
sudo systemctl enable shutdown-button.service
echo -e "${GREEN}Service enabled${NC}"

# Start service
echo -e "${YELLOW}Starting service...${NC}"
sudo systemctl start shutdown-button.service

# Check status
echo ""
echo -e "${BLUE}Service status:${NC}"
sudo systemctl status shutdown-button.service --no-pager -l

echo ""
echo -e "${GREEN}Shutdown Button Service installed successfully!${NC}"
echo ""
echo -e "${YELLOW}Useful commands:${NC}"
echo -e "   Status:  ${BLUE}sudo systemctl status shutdown-button.service${NC}"
echo -e "   Logs:    ${BLUE}sudo journalctl -u shutdown-button.service -f${NC}"
echo -e "   Restart: ${BLUE}sudo systemctl restart shutdown-button.service${NC}"
echo -e "   Stop:    ${BLUE}sudo systemctl stop shutdown-button.service${NC}"
