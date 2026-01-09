# WhatsApp Web.js API (Terminal + Postman + Session Persisted)

This project is a **single-process WhatsApp automation API** built using **Node.js**, **Express**, and **whatsapp-web.js**.
It supports **WhatsApp Business App**, works in **India**, and allows sending messages via **Postman, curl, or terminal scripts** without re-scanning QR on every restart.

> ⚠️ This uses **WhatsApp Web automation (unofficial)**. Use responsibly.

---

## 1. Features

* WhatsApp Business App support
* Persistent login (scan QR **only once**)
* Single POST API to send:

  * Text
  * Links
  * Images (via URL)
* Terminal support using `curl`
* Message loop with delay (safe)
* Works with Indian numbers (+91)

---

## 2. Tech Stack

* Node.js (v18+ recommended)
* Express.js
* whatsapp-web.js
* Puppeteer (bundled)
* qrcode-terminal

---

## 3. Project Structure

```
wwebjs-bot/
├── server.js
├── package.json
├── .wwebjs_auth/        # Auto-created (DO NOT DELETE)
└── README.md
```

---

## 4. Installation

```bash
cd ~/Whatsapp/wwebjs-bot
npm install express whatsapp-web.js qrcode-terminal
```

---

## 5. Server Code (Single File)

The entire logic runs in **one file**: `server.js`

### Key points:

* Uses `LocalAuth` to store session
* Blocks API calls until WhatsApp is ready
* Only **one Node process** must run

---

## 6. Start the Server

```bash
node server.js
```

### First run only

* QR code will appear in terminal
* Scan using **WhatsApp Business App → Linked Devices**
* Wait for:

```
WhatsApp client is ready
API running at http://localhost:3000
```

### Next runs

* **No QR scan required**

---

## 7. API Endpoint (Single POST)

### Endpoint

```
POST http://localhost:3000/send
```

---

### Send Text / Link

```json
{
  "number": "919663734572",
  "message": "Hello from India"
}
```

Links can be included inside `message`.

---

### Send Image

```json
{
  "number": "919663734572",
  "imageUrl": "https://example.com/image.jpg",
  "caption": "Optional caption"
}
```

---

## 8. Terminal Usage (curl)

```bash
curl -X POST http://localhost:3000/send \
  -H "Content-Type: application/json" \
  -d '{
    "number": "919663734572",
    "message": "Hello from terminal"
  }'
```

Expected response:

```json
{"status":"sent","type":"text/link"}
```

---

## 9. Loop Sending (10 Emotional Messages)

### Terminal Loop (Safe)

```bash
messages=(
  "You are stronger than you think. Keep going."
  "Every bad day prepares you for a better tomorrow."
  "Do not give up. Great things take time."
  "Your struggles are shaping your future."
  "Even the darkest night ends with sunrise."
  "Believe in yourself."
  "Pain is temporary, growth is permanent."
  "You are doing better than you realize."
  "Small steps lead to big changes."
  "Stay patient. Your time will come."
)

for msg in "${messages[@]}"
do
  curl -s -X POST http://localhost:3000/send \
    -H "Content-Type: application/json" \
    -d "{\"number\":\"919663734572\",\"message\":\"$msg\"}"

  sleep 2
done
```

---

## 10. WhatsApp Safety Rules (IMPORTANT)

* Always include country code (`91` for India)
* Add **2–3 sec delay** between messages
* Avoid bulk spam
* Avoid unknown numbers
* Do NOT run multiple Node instances

---

## 11. Session Persistence (No QR Again)

Session is stored automatically in:

```
.wwebjs_auth/session-main-session/
```

Do NOT:

* Delete this folder
* Change `clientId`
* Run server from another directory

---

## 12. Supported

| Feature               | Status |
| --------------------- | ------ |
| WhatsApp Business App | ✅      |
| India numbers         | ✅      |
| Unicode languages     | ✅      |
| Terminal              | ✅      |
| Postman               | ✅      |
| Session persistence   | ✅      |

---

## 13. Disclaimer

This project uses **unofficial WhatsApp Web automation**.

* Not recommended for marketing
* Not Meta-approved
* Use for learning, internal tools, or personal automation

---

## 14. Author

**Rachappa Biradar**
India

---

## 15. Next Improvements (Optional)

* API key authentication
* Rate limiting
* PM2 auto-restart
* Docker support
* Bulk CSV sender
* Daily scheduled messages

---

✅ Project setup complete and working.
