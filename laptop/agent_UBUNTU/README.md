Below is a **complete, professional README.md** you can **copy-paste directly** into your project root.

It documents **everything you built**, explains **how it works**, and gives **step-by-step install & run instructions**, including **virtual environment setup**.

---

# 🧠 Ubuntu Productivity Agent

A **lightweight, privacy-respecting Ubuntu activity tracking agent** that observes **what apps you use, for how long, your focus patterns, and background media usage**, and stores **15-minute summaries in MongoDB Atlas (cloud)**.

This project is designed for **habit understanding and productivity analysis**, not surveillance.

---

## ✨ Features

### ✅ Core Features

* **Per-second foreground app sampling**
* **Exact time spent per app**
* **Keyboard & mouse activity counts** (counts only, no content)
* **System state tracking**

  * Active
  * Idle
  * Locked
* **Focus streak detection**
* **Time-of-day classification**
* **Background media detection**

  * Detects if video/music is playing
  * Identifies source (browser, VLC, Spotify, etc.)
  * Captures media title (when available)
* **Cloud storage using MongoDB Atlas**

### ❌ What This Agent Does NOT Do

* No keystroke logging
* No screen capture
* No audio recording
* No browser history or URLs
* No CPU / memory monitoring
* No machine learning

---

## 🗂️ Project Structure

```
productivity-agent/
│
├── agent/
│   ├── collectors/
│   │   ├── input_tracker.py      # keyboard & mouse counts
│   │   ├── app_tracker.py        # foreground app detection
│   │   ├── power_tracker.py      # active / idle / locked
│   │   └── media_tracker.py      # background media + title
│   │
│   ├── aggregator/
│   │   └── window_15m.py         # 15-minute aggregation logic
│   │
│   ├── writer/
│   │   └── mongo_writer.py       # MongoDB Atlas writer
│   │
│   └── main.py                   # main agent loop
│
├── config.yaml                   # configuration
├── requirements.txt              # Python dependencies
└── README.md
```

---

## 🧩 Data Model (Stored Every 15 Minutes)

Example MongoDB document:

```json
{
  "timestamp": "2026-01-09T16:15:00Z",
  "device": "ubuntu_laptop",
  "user": "rachappa",
  "os": "ubuntu",
  "time_bucket": "afternoon",

  "keys_count": 214,
  "mouse_count": 502,

  "active_seconds": 840,
  "idle_seconds": 60,
  "locked_seconds": 0,

  "apps": {
    "Code": 720,
    "Terminal": 120,
    "Chrome": 60
  },

  "app_switch_count": 3,
  "max_focus_streak_seconds": 420,

  "background_media": true,
  "background_media_source": "browser_video",
  "background_media_title": "How Linux Scheduling Works",
  "background_media_seconds": 420
}
```

---

## 🖥️ System Requirements

* **Ubuntu 20.04+**
* X11 session (Wayland works partially)
* Python **3.9+**
* Internet connection (for MongoDB Atlas)

---

## 📦 System Dependencies (Ubuntu)

Install required system tools:

```bash
sudo apt update
sudo apt install -y \
  python3 python3-pip python3-venv \
  xdotool wmctrl \
  playerctl
```

Verify media support:

```bash
playerctl status
```

> If no media is playing, you may see:
> `No player could handle this command` — this is normal.

---

## 🐍 Python Virtual Environment Setup

### 1️⃣ Create virtual environment

```bash
cd productivity-agent
python3 -m venv venv
```

### 2️⃣ Activate virtual environment

```bash
source venv/bin/activate
```

You should see:

```text
(venv) user@laptop:~/productivity-agent$
```

---

## 📦 Python Dependencies

### `requirements.txt`

```txt
pynput
pymongo
dnspython
pyyaml
```

### Install dependencies

```bash
pip install -r requirements.txt
```

---

## ☁️ MongoDB Atlas Setup (Cloud)

### 1️⃣ Create MongoDB Atlas account

* [https://www.mongodb.com/cloud/atlas](https://www.mongodb.com/cloud/atlas)
* Create **Free Tier (M0)** cluster

### 2️⃣ Database Access

* Create a database user with **read/write** permissions

### 3️⃣ Network Access

* Add your IP address
  *(or temporarily allow `0.0.0.0/0` for testing)*

### 4️⃣ Get Connection URI

Example:

```
mongodb+srv://USERNAME:PASSWORD@cluster0.xxxxx.mongodb.net/?retryWrites=true&w=majority
```

---

## ⚙️ Configuration

### `config.yaml`

```yaml
device:
  name: ubuntu_laptop
  user: rachappa
  os: ubuntu

interval:
  window_seconds: 900   # 15 minutes (use 10 for testing)

mongodb:
  uri: "mongodb+srv://USERNAME:PASSWORD@cluster0.xxxxx.mongodb.net/?retryWrites=true&w=majority"
  database: "productivity"
  collection: "activity_15m"
```

⚠️ **Do not commit `config.yaml` to GitHub** (contains credentials).

---

## ▶️ Running the Agent

### Run manually (recommended for testing)

```bash
source venv/bin/activate
python3 agent/main.py
```

Expected output:

```text
MongoDB Atlas connected
Ubuntu productivity agent started
Per-second app sampling + background media + title enabled
15-minute window written to MongoDB Atlas
```

---

## 🧪 Testing Mode

For fast testing, set in `config.yaml`:

```yaml
interval:
  window_seconds: 10
```

You will see data written every 10 seconds.

---

## ⚠️ Important Notes

### VS Code Terminal Limitation

* `playerctl` **does NOT work reliably** inside VS Code integrated terminal
* Run the agent from:

  * Ubuntu system terminal
  * systemd user service (recommended for production)

This is a **Linux DBus environment issue**, not a bug.

---

## 🧠 How the Agent Works (Simple Explanation)

* **Every second**

  * Detects focused app
  * Checks system state
  * Checks if media is playing
* **Continuously**

  * Counts keyboard & mouse events
* **Every 15 minutes**

  * Aggregates all data
  * Writes ONE document to MongoDB
  * Resets counters

This design:

* Minimizes storage
* Preserves privacy
* Accurately represents user behavior

---

## 🔐 Privacy & Ethics

This project is designed to be:

* Transparent
* Minimal
* Ethical

It tracks **attention and behavior patterns**, not content.

---

## 🚀 Next Possible Extensions (Optional)

* systemd user service (auto-start on login)
* Daily habit summary generator
* CLI: “What did I do in last 15 minutes?”
* Dashboard / visualization
* Correlation with IoT / ESP32 data

---

## 📄 License

For personal, educational, and research use.

---

If you want, next I can:

* Create a **systemd user service file**
* Add a **daily summary script**
* Help you **publish this project cleanly on GitHub**

Just tell me what you want next.
