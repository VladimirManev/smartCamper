#!/bin/bash
# Deploy SmartCamper to Raspberry Pi via SSH
# Скрипт за качване на проекта на Raspberry Pi през SSH
#
# ============================================================================
# ИНСТРУКЦИИ ЗА ИЗПОЛЗВАНЕ:
# ============================================================================
#
# 1. ПРЕДИ ПЪРВО ИЗПОЛЗВАНЕ:
#    - Увери се, че си свързан към WiFi мрежата на Raspberry Pi (обикновено "SmartCamper")
#    - Провери дали Raspberry Pi е включен и достъпен: ping 192.168.4.1
#    - Провери SSH достъпа: ssh vmanev@192.168.4.1
#
# 2. КОНФИГУРАЦИЯ (промени стойностите по-долу):
#    - RASPBERRY_IP: IP адрес на Pi (обикновено 192.168.4.1)
#    - RASPBERRY_USER: Потребителско име (обикновено vmanev)
#    - RASPBERRY_PATH: Път към проекта (обикновено ~/smartCamper)
#
# 3. ИЗПОЛЗВАНЕ:
#    От директорията на проекта изпълни:
#    ./deploy-to-raspberry.sh
#
#    Скриптът ще:
#    - Тества SSH връзката
#    - Качи променените файлове (backend, frontend, root файлове)
#    - Автоматично изпълни update-from-local.sh на Pi
#    - Инсталира зависимости и направи build на frontend
#    - Рестартира backend service
#
# 4. ОПЦИИ:
#    ./deploy-to-raspberry.sh --build-frontend    # Build на frontend преди качване
#    ./deploy-to-raspberry.sh --skip-frontend    # Пропусни frontend
#    ./deploy-to-raspberry.sh --skip-backend      # Пропусни backend
#    ./deploy-to-raspberry.sh --password ПАРОЛА  # Задай SSH парола
#    ./deploy-to-raspberry.sh --help              # Покажи помощ
#
# 5. ЗА БЕЗПАРОЛЕН ДОСТЪП (препоръчително):
#    Настрой SSH ключове автоматично:
#    ./setup-ssh-keys.sh
#
#    Или ръчно:
#    ssh-copy-id vmanev@192.168.4.1
#
#    Или инсталирай sshpass:
#    brew install hudochenkov/sshpass/sshpass
#
# 6. АКО ИМА ПРОБЛЕМИ:
#    - Провери дали си свързан към правилната WiFi мрежа
#    - Провери дали Raspberry Pi е включен
#    - Провери дали SSH е активиран на Pi
#    - Ако има проблеми с node_modules, използвай: ./clean-node-modules-on-pi.sh
#
# ============================================================================

set -e  # Stop on error

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration - ПРОМЕНИ ТЕЗИ СТОЙНОСТИ!
RASPBERRY_IP="192.168.4.1"  # IP адрес на Raspberry Pi (обикновено 192.168.4.1 за WiFi AP)
RASPBERRY_USER="vmanev"      # Потребителско име на Raspberry Pi
RASPBERRY_PATH="~/smartCamper"  # Път към проекта на Raspberry Pi
SSH_KEY=""                    # Път към SSH ключ (ако е нужно, напр. ~/.ssh/id_rsa)
                              # Ако е празно, скриптът ще опита да намери автоматично
SSH_PASSWORD=""               # SSH парола (ако не използваш SSH ключ)

# Auto-detect SSH key if not specified
if [ -z "$SSH_KEY" ]; then
    # Try common SSH key locations
    if [ -f "$HOME/.ssh/id_rsa" ]; then
        SSH_KEY="$HOME/.ssh/id_rsa"
    elif [ -f "$HOME/.ssh/id_ed25519" ]; then
        SSH_KEY="$HOME/.ssh/id_ed25519"
    elif [ -f "$HOME/.ssh/id_ecdsa" ]; then
        SSH_KEY="$HOME/.ssh/id_ecdsa"
    fi
fi

# Parse command line arguments
BUILD_FRONTEND=false
SKIP_FRONTEND=false
SKIP_BACKEND=false
OFFLINE_PI=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --build-frontend)
            BUILD_FRONTEND=true
            shift
            ;;
        --skip-frontend)
            SKIP_FRONTEND=true
            shift
            ;;
        --skip-backend)
            SKIP_BACKEND=true
            shift
            ;;
        --offline-pi)
            OFFLINE_PI=true
            shift
            ;;
        --ip)
            RASPBERRY_IP="$2"
            shift 2
            ;;
        --user)
            RASPBERRY_USER="$2"
            shift 2
            ;;
        --password)
            SSH_PASSWORD="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --build-frontend    Build frontend locally before deploying"
            echo "  --skip-frontend     Skip deploying frontend files"
            echo "  --skip-backend      Skip deploying backend files"
            echo "  --offline-pi        Build frontend locally and skip frontend build on Pi"
            echo "  --ip IP             Raspberry Pi IP address (default: $RASPBERRY_IP)"
            echo "  --user USER         Raspberry Pi username (default: $RASPBERRY_USER)"
            echo "  --password PASS     SSH password (will prompt if not provided)"
            echo "  --help              Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}🚀 Deploying SmartCamper to Raspberry Pi...${NC}"
echo -e "${YELLOW}📍 Target: ${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}${NC}"
echo ""

if [ "$OFFLINE_PI" = true ]; then
    BUILD_FRONTEND=true
    echo -e "${YELLOW}📴 Offline Pi mode enabled: frontend will be built locally and Pi build will be skipped.${NC}"
    echo ""
fi

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$SCRIPT_DIR"

# Check if rsync is available
if ! command -v rsync &> /dev/null; then
    echo -e "${RED}❌ Error: rsync is not installed!${NC}"
    echo "Install it with: brew install rsync (on macOS) or apt-get install rsync (on Linux)"
    exit 1
fi

# Start ssh-agent and add key if needed
if [ -n "$SSH_KEY" ] && [ -f "$SSH_KEY" ]; then
    # Check if ssh-agent is running
    if [ -z "$SSH_AUTH_SOCK" ]; then
        eval "$(ssh-agent -s)" > /dev/null
    fi
    
    # Check if key is already added to ssh-agent
    if ! ssh-add -l 2>/dev/null | grep -q "$SSH_KEY"; then
        echo -e "${YELLOW}🔑 Adding SSH key to ssh-agent...${NC}"
        echo -e "${YELLOW}   You will be asked for your key passphrase once${NC}"
        ssh-add "$SSH_KEY" 2>/dev/null || {
            echo -e "${YELLOW}⚠️  Could not add key to ssh-agent. You may be asked for passphrase multiple times.${NC}"
        }
    fi
    echo -e "${GREEN}🔑 Using SSH key: $SSH_KEY${NC}"
fi

# Build initial SSH command (try without password first)
SSH_CMD="ssh"
if [ -n "$SSH_KEY" ] && [ -f "$SSH_KEY" ]; then
    SSH_CMD="$SSH_CMD -i $SSH_KEY"
fi
SSH_CMD="$SSH_CMD -o StrictHostKeyChecking=no -o ConnectTimeout=10"

# Check if sshpass is available
USE_SSHPASS=false
if command -v sshpass &> /dev/null; then
    USE_SSHPASS=true
fi

# Test SSH connection
echo -e "${YELLOW}🔌 Testing SSH connection...${NC}"
if ! $SSH_CMD "${RASPBERRY_USER}@${RASPBERRY_IP}" "echo 'Connection successful'" &> /dev/null; then
    # Connection failed - might need password
    # If we have SSH key but it didn't work, try without it (might not be authorized)
    if [ -n "$SSH_KEY" ] && [ -f "$SSH_KEY" ]; then
        echo -e "${YELLOW}⚠️  SSH key found but connection failed.${NC}"
        echo -e "${YELLOW}   The key might not be authorized on Raspberry Pi.${NC}"
        echo -e "${YELLOW}   Run ./setup-ssh-keys.sh to set up SSH keys.${NC}"
        echo ""
    fi
    
    if [ -z "$SSH_PASSWORD" ]; then
        echo -e "${YELLOW}⚠️  SSH connection failed. Password required.${NC}"
        if [ "$USE_SSHPASS" = false ]; then
            echo -e "${YELLOW}   Note: sshpass is not installed. You can install it with:${NC}"
            echo -e "${YELLOW}   brew install hudochenkov/sshpass/sshpass (on macOS)${NC}"
            echo ""
        fi
        read -sp "Enter SSH password for ${RASPBERRY_USER}@${RASPBERRY_IP}: " SSH_PASSWORD
        echo ""
        
        # Update SSH command with password
        if [ "$USE_SSHPASS" = true ] && [ -n "$SSH_PASSWORD" ]; then
            SSH_CMD="sshpass -p '$SSH_PASSWORD' ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10"
        else
            # Without sshpass, we can't use password in script - user needs to install it
            echo -e "${RED}❌ Error: Cannot use password without sshpass.${NC}"
            echo -e "${YELLOW}   Please install sshpass or set up SSH key authentication.${NC}"
            exit 1
        fi
        
        # Test again with password
        if ! eval "$SSH_CMD ${RASPBERRY_USER}@${RASPBERRY_IP} 'echo Connection successful'" &> /dev/null; then
            echo -e "${RED}❌ Error: Cannot connect to Raspberry Pi even with password!${NC}"
            echo -e "${YELLOW}   Make sure:${NC}"
            echo -e "${YELLOW}   - Password is correct${NC}"
            echo -e "${YELLOW}   - Raspberry Pi is powered on and connected${NC}"
            echo -e "${YELLOW}   - You are connected to the WiFi network (SmartCamper)${NC}"
            echo -e "${YELLOW}   - SSH is enabled on Raspberry Pi${NC}"
            echo -e "${YELLOW}   - IP address is correct: ${RASPBERRY_IP}${NC}"
            exit 1
        fi
    elif [ -n "$SSH_PASSWORD" ]; then
        # Password was provided via --password flag
        if [ "$USE_SSHPASS" = true ]; then
            SSH_CMD="sshpass -p '$SSH_PASSWORD' ssh -o StrictHostKeyChecking=no -o ConnectTimeout=10"
            if ! eval "$SSH_CMD ${RASPBERRY_USER}@${RASPBERRY_IP} 'echo Connection successful'" &> /dev/null; then
                echo -e "${RED}❌ Error: Cannot connect to Raspberry Pi!${NC}"
                echo -e "${YELLOW}   Make sure password is correct and Raspberry Pi is accessible.${NC}"
                exit 1
            fi
        else
            echo -e "${RED}❌ Error: sshpass is required when using --password option.${NC}"
            echo -e "${YELLOW}   Install it with: brew install hudochenkov/sshpass/sshpass (on macOS)${NC}"
            exit 1
        fi
    else
        echo -e "${RED}❌ Error: Cannot connect to Raspberry Pi!${NC}"
        echo -e "${YELLOW}   Make sure:${NC}"
        echo -e "${YELLOW}   - Raspberry Pi is powered on and connected${NC}"
        echo -e "${YELLOW}   - You are connected to the WiFi network (SmartCamper)${NC}"
        echo -e "${YELLOW}   - SSH is enabled on Raspberry Pi${NC}"
        echo -e "${YELLOW}   - IP address is correct: ${RASPBERRY_IP}${NC}"
        echo -e "${YELLOW}   - SSH key is set up or password is provided${NC}"
        exit 1
    fi
fi
echo -e "${GREEN}✅ SSH connection successful!${NC}"
echo ""

# Build frontend if requested
if [ "$BUILD_FRONTEND" = true ]; then
    echo -e "${YELLOW}🔨 Building frontend locally...${NC}"
    cd "$PROJECT_DIR/frontend"
    
    if [ ! -d "node_modules" ]; then
        echo -e "${YELLOW}📥 Installing frontend dependencies...${NC}"
        npm install
    fi
    
    echo -e "${GREEN}🔨 Building React application...${NC}"
    npm run build
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Build successful!${NC}"
    else
        echo -e "${RED}❌ Build failed!${NC}"
        exit 1
    fi
    
    cd "$PROJECT_DIR"
    echo ""
fi

# Create rsync exclude file
RSYNC_EXCLUDE_FILE=$(mktemp)
cat > "$RSYNC_EXCLUDE_FILE" << EOF
# Dependencies
node_modules/
npm-debug.log*
yarn-debug.log*
yarn-error.log*

# Production builds (will be built on Pi)
frontend/dist/
backend/build/

# Environment variables
.env
.env.local
.env.development.local
.env.test.local
.env.production.local

# IDE files
.vscode/
.idea/
*.swp
*.swo

# OS files
.DS_Store
Thumbs.db

# Logs
logs/
*.log

# Git
.git/
.gitignore

# Temporary files
tmp/
temp/
*.tmp

# Documentation (optional - remove if you want to deploy docs)
*.md
!README.md
!README_BG.md
EOF

# Build SSH options for rsync
if [ "$USE_SSHPASS" = true ] && [ -n "$SSH_PASSWORD" ]; then
    RSYNC_SSH_OPTS="-e \"sshpass -p '$SSH_PASSWORD' ssh -o StrictHostKeyChecking=no\""
elif [ -n "$SSH_KEY" ]; then
    RSYNC_SSH_OPTS="-e \"ssh -i $SSH_KEY -o StrictHostKeyChecking=no\""
else
    RSYNC_SSH_OPTS="-e \"ssh -o StrictHostKeyChecking=no\""
fi

# Base rsync options
RSYNC_BASE_OPTS="-avz --progress --delete --exclude-from=$RSYNC_EXCLUDE_FILE"

# Deploy files
echo -e "${GREEN}📤 Uploading files to Raspberry Pi...${NC}"
echo ""

# Deploy backend
if [ "$SKIP_BACKEND" = false ]; then
    echo -e "${BLUE}📦 Deploying backend...${NC}"
    eval rsync $RSYNC_BASE_OPTS $RSYNC_SSH_OPTS \
        "$PROJECT_DIR/backend/" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/backend/"
    echo ""
fi

# Deploy frontend source (not dist, will be built on Pi)
if [ "$SKIP_FRONTEND" = false ]; then
    echo -e "${BLUE}📦 Deploying frontend source...${NC}"
    eval rsync $RSYNC_BASE_OPTS $RSYNC_SSH_OPTS \
        --exclude 'dist/' \
        "$PROJECT_DIR/frontend/" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/frontend/"
    echo ""

    if [ "$OFFLINE_PI" = true ]; then
        echo -e "${BLUE}📦 Deploying frontend dist (offline Pi mode)...${NC}"
        eval rsync -avz --progress $RSYNC_SSH_OPTS --delete \
            "$PROJECT_DIR/frontend/dist/" \
            "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/frontend/dist/"
        echo ""
    fi
fi

# Deploy root files (only specific files)
echo -e "${BLUE}📦 Deploying root files...${NC}"
eval rsync -avz --progress $RSYNC_SSH_OPTS \
    "$PROJECT_DIR/update-from-local.sh" \
    "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/"

if [ -f "$PROJECT_DIR/smartcamper-backend.service" ]; then
    eval rsync -avz --progress $RSYNC_SSH_OPTS \
        "$PROJECT_DIR/smartcamper-backend.service" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/"
fi

if [ -f "$PROJECT_DIR/hostapd.conf" ]; then
    eval rsync -avz --progress $RSYNC_SSH_OPTS \
        "$PROJECT_DIR/hostapd.conf" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/"
fi

if [ -f "$PROJECT_DIR/shutdown-button.py" ]; then
    eval rsync -avz --progress $RSYNC_SSH_OPTS \
        "$PROJECT_DIR/shutdown-button.py" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/"
fi

if [ -f "$PROJECT_DIR/shutdown-button.service" ]; then
    eval rsync -avz --progress $RSYNC_SSH_OPTS \
        "$PROJECT_DIR/shutdown-button.service" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/"
fi

if [ -f "$PROJECT_DIR/install-shutdown-button.sh" ]; then
    eval rsync -avz --progress $RSYNC_SSH_OPTS \
        "$PROJECT_DIR/install-shutdown-button.sh" \
        "${RASPBERRY_USER}@${RASPBERRY_IP}:${RASPBERRY_PATH}/"
fi
echo ""

# Clean up exclude file
rm -f "$RSYNC_EXCLUDE_FILE"

# Run update script on Raspberry Pi
echo -e "${GREEN}🔄 Running update script on Raspberry Pi...${NC}"
REMOTE_ENV=""
if [ "$OFFLINE_PI" = true ]; then
    REMOTE_ENV="SKIP_FRONTEND_BUILD=true "
fi

if [[ "$SSH_CMD" == *"sshpass"* ]]; then
    eval "$SSH_CMD ${RASPBERRY_USER}@${RASPBERRY_IP} 'cd ${RASPBERRY_PATH} && chmod +x update-from-local.sh && ${REMOTE_ENV}./update-from-local.sh'"
else
    $SSH_CMD "${RASPBERRY_USER}@${RASPBERRY_IP}" "cd ${RASPBERRY_PATH} && chmod +x update-from-local.sh && ${REMOTE_ENV}./update-from-local.sh"
fi

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✨ Deployment completed successfully!${NC}"
    echo -e "${YELLOW}📡 Application is available at: http://${RASPBERRY_IP}:3000${NC}"
else
    echo ""
    echo -e "${RED}❌ Deployment completed but update script failed!${NC}"
    echo -e "${YELLOW}   Check the output above for errors.${NC}"
    exit 1
fi
