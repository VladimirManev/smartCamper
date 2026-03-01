# Deployment Instructions - SmartCamper

## 🚀 Quick Start

### Step 1: Preparation
1. Make sure you're connected to Raspberry Pi's WiFi network (usually "SmartCamper")
2. Check if Raspberry Pi is powered on: `ping 192.168.4.1`
3. Test SSH access: `ssh vmanev@192.168.4.1`

### Step 2: Deploy the project
From the project directory, run:

```bash
./deploy-to-raspberry.sh
```

The script automatically:
- ✅ Tests SSH connection
- ✅ Uploads changed files (backend, frontend)
- ✅ Installs dependencies
- ✅ Builds frontend
- ✅ Restarts backend service

## 📋 Options

```bash
# Build frontend before deploying (faster on Pi)
./deploy-to-raspberry.sh --build-frontend

# Skip frontend (backend only)
./deploy-to-raspberry.sh --skip-frontend

# Skip backend (frontend only)
./deploy-to-raspberry.sh --skip-backend

# Set SSH password
./deploy-to-raspberry.sh --password your_password

# Help
./deploy-to-raspberry.sh --help
```

## 🔧 Configuration

Open `deploy-to-raspberry.sh` and modify at the beginning of the file:

```bash
RASPBERRY_IP="192.168.4.1"    # Raspberry Pi IP address
RASPBERRY_USER="vmanev"       # Username
RASPBERRY_PATH="~/smartCamper" # Project path
```

## 🔐 Passwordless Access (Recommended)

### Easiest way: Automatic SSH key setup
```bash
./setup-ssh-keys.sh
```

The script automatically:
- ✅ Generates SSH key (if you don't have one)
- ✅ Copies key to Raspberry Pi
- ✅ Tests passwordless access

**Important:** If you set a passphrase (password) for your SSH key:
- On first run of `deploy-to-raspberry.sh` you'll be asked for passphrase once
- The script automatically adds the key to `ssh-agent`, which caches the passphrase
- After that, it won't ask for passphrase on every command

**For completely passwordless access:**
- When generating the key, press Enter for empty passphrase
- Or manually add key to ssh-agent: `ssh-add ~/.ssh/id_rsa`

### Alternative 1: Manual SSH key setup
```bash
# Generate SSH key (if you don't have one)
ssh-keygen -t rsa -b 4096

# Copy key to Raspberry Pi
ssh-copy-id vmanev@192.168.4.1

# Add key to ssh-agent (to avoid entering passphrase)
ssh-add ~/.ssh/id_rsa
```

### Alternative 2: Use sshpass
```bash
# Install on macOS
brew install hudochenkov/sshpass/sshpass

# Then use with --password option
./deploy-to-raspberry.sh --password your_password
```

### Why ssh-agent?

If your SSH key has a passphrase, `ssh-agent` caches the password for the current session:
- Enter passphrase once at startup
- After that, all SSH commands work without asking for password
- The `deploy-to-raspberry.sh` script automatically uses `ssh-agent`

## 🛠️ Troubleshooting

### Problem: Cannot connect via SSH
**Solution:**
1. Check if you're connected to the correct WiFi network
2. Check IP address: `ping 192.168.4.1`
3. Check if SSH is enabled on Pi: `sudo systemctl status ssh`

### Problem: npm install errors (ENOTEMPTY)
**Solution:**
```bash
# Clean node_modules on Pi
./clean-node-modules-on-pi.sh

# Or manually:
ssh vmanev@192.168.4.1
cd ~/smartCamper
rm -rf frontend/node_modules backend/node_modules
```

### Problem: Node.js version is old
**Solution:**
```bash
# Update Node.js on Pi
./update-nodejs-on-pi.sh

# Or manually:
ssh vmanev@192.168.4.1
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt install -y nodejs
```

### Problem: Build fails
**Solution:**
1. Check Node.js version: `node --version` (should be 20.x or 22.x)
2. Check logs: `sudo journalctl -u smartcamper-backend -n 50`
3. Try manual build: 
   ```bash
   ssh vmanev@192.168.4.1
   cd ~/smartCamper/frontend
   rm -rf node_modules
   npm install
   npm run build
   ```

## 📝 Common Commands

```bash
# Deploy changes
./deploy-to-raspberry.sh

# Clean node_modules on Pi
./clean-node-modules-on-pi.sh

# Update Node.js on Pi
./update-nodejs-on-pi.sh

# Check backend status on Pi
ssh vmanev@192.168.4.1 "sudo systemctl status smartcamper-backend"

# View backend logs
ssh vmanev@192.168.4.1 "sudo journalctl -u smartcamper-backend -n 50"
```

## ✅ After Successful Deployment

The application is available at: **http://192.168.4.1:3000**

You can open it in your browser and see the changes.

## 📌 Notes

- The script only uploads changed files (rsync)
- `node_modules` and `dist` are not uploaded (installed/built on Pi)
- Backend service is automatically restarted after changes
- If you have problems, see the "Troubleshooting" section above
