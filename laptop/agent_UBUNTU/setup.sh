#!/usr/bin/env bash
set -e

echo "=========================================="
echo " Ubuntu Productivity Agent – Auto Setup"
echo "=========================================="

# -----------------------------
# CONFIG (CHANGE IF NEEDED)
# -----------------------------
PROJECT_DIR="$HOME/klmn/laptop/agent_UBUNTU"
VENV_DIR="$PROJECT_DIR/venv"
SERVICE_NAME="productivity-agent"
SERVICE_FILE="$HOME/.config/systemd/user/${SERVICE_NAME}.service"

# -----------------------------
# 1. CHECK OS
# -----------------------------
if [[ "$(uname -s)" != "Linux" ]]; then
  echo "❌ This script is for Linux/Ubuntu only."
  exit 1
fi

# -----------------------------
# 2. INSTALL SYSTEM PACKAGES
# -----------------------------
echo "➡ Installing system packages..."

sudo apt update
sudo apt install -y \
  python3 \
  python3-pip \
  python3-venv \
  xdotool \
  wmctrl \
  playerctl

echo "✔ System packages installed"

# -----------------------------
# 3. CHECK PROJECT DIRECTORY
# -----------------------------
if [[ ! -d "$PROJECT_DIR" ]]; then
  echo "❌ Project directory not found:"
  echo "   $PROJECT_DIR"
  exit 1
fi

cd "$PROJECT_DIR"

echo "✔ Project directory found: $PROJECT_DIR"

# -----------------------------
# 4. CREATE VIRTUAL ENV
# -----------------------------
if [[ ! -d "$VENV_DIR" ]]; then
  echo "➡ Creating Python virtual environment..."
  python3 -m venv venv
else
  echo "✔ Virtual environment already exists"
fi

# -----------------------------
# 5. ACTIVATE VENV
# -----------------------------
source "$VENV_DIR/bin/activate"

echo "✔ Virtual environment activated"

# -----------------------------
# 6. UPGRADE PIP
# -----------------------------
pip install --upgrade pip

# -----------------------------
# 7. INSTALL PYTHON DEPENDENCIES
# -----------------------------
if [[ ! -f "requirements.txt" ]]; then
  echo "❌ requirements.txt not found"
  exit 1
fi

echo "➡ Installing Python dependencies..."
pip install -r requirements.txt

echo "✔ Python dependencies installed"

# -----------------------------
# 8. QUICK DEPENDENCY TEST
# -----------------------------
python3 - <<EOF
import pynput, pymongo, yaml
print("✔ Python dependency test passed")
EOF

# -----------------------------
# 9. OPTIONAL: SYSTEMD USER SERVICE
# -----------------------------
echo "➡ Setting up systemd user service..."

mkdir -p "$HOME/.config/systemd/user"

cat > "$SERVICE_FILE" <<EOL
[Unit]
Description=Ubuntu Productivity Agent
After=graphical-session.target

[Service]
Type=simple
WorkingDirectory=$PROJECT_DIR
ExecStart=$VENV_DIR/bin/python $PROJECT_DIR/main.py
Restart=on-failure
RestartSec=10
Environment=PYTHONUNBUFFERED=1

[Install]
WantedBy=default.target
EOL

echo "✔ systemd service file created"

# -----------------------------
# 10. ENABLE & START SERVICE
# -----------------------------
systemctl --user daemon-reload
systemctl --user enable "$SERVICE_NAME"
systemctl --user restart "$SERVICE_NAME"

echo "✔ systemd user service enabled and started"

# -----------------------------
# 11. FINAL STATUS
# -----------------------------
echo "------------------------------------------"
echo " Setup complete 🎉"
echo "------------------------------------------"
echo " Service status:"
systemctl --user status "$SERVICE_NAME" --no-pager

echo
echo " Logs:"
echo "   journalctl --user -u $SERVICE_NAME -f"
echo
echo " Done."
