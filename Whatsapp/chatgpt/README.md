Below is a **clean, professional `README.md`** suitable for GitHub. It explains the purpose, setup, environment variables, and usage of your AI chat backend clearly.

---

# AI Game Character Chat Server

This project is a **Node.js + Express backend** that connects a **female game-character AI persona** to WhatsApp (or any messaging service) using the **OpenRouter AI API**.
It accepts user messages, forwards them to an AI model, and sends the AI-generated reply back to WhatsApp.

---

## Features

* Express-based REST API
* AI chat using **OpenRouter** (`mistralai/mistral-7b-instruct`)
* Custom system prompt (female game character personality)
* WhatsApp message forwarding
* CORS enabled
* Environment-based configuration
* Detailed request and error logging

---

## Tech Stack

* **Node.js**
* **Express.js**
* **Axios**
* **dotenv**
* **OpenRouter AI**
* **WhatsApp API (custom endpoint)**

---

## Project Structure

```
.
├── server.js          # Main server file
├── .env               # Environment variables
├── package.json
└── README.md
```

---

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/ai-game-chat-server.git
cd ai-game-chat-server
```

### 2. Install Dependencies

```bash
npm install
```

---

## Environment Variables

Create a `.env` file in the root directory:

```env
OPENROUTER_API_KEY=your_openrouter_api_key
WHATSAPP_API=https://your-whatsapp-api-endpoint
```

### Variable Explanation

| Variable             | Description                                                 |
| -------------------- | ----------------------------------------------------------- |
| `OPENROUTER_API_KEY` | API key from [https://openrouter.ai](https://openrouter.ai) |
| `WHATSAPP_API`       | Endpoint that sends messages to WhatsApp                    |

---

## Running the Server

```bash
node server.js
```

Server will start at:

```
http://localhost:4000
```

---

## API Endpoint

### `POST /chat`

Sends a user message to the AI and forwards the reply to WhatsApp.

#### Request Body

```json
{
  "number": "919876543210",
  "message": "Hello, who are you?"
}
```

#### Response

```json
{
  "status": "success",
  "reply": "Hi! I'm your game companion. Ready for an adventure?"
}
```

---

## AI Configuration

The AI personality is controlled by this system prompt:

```js
{
  role: 'system',
  content: 'You are a friendly female game character.'
}
```

You can customize this to:

* RPG character
* Anime-style character
* Sci-fi assistant
* Companion NPC

---

## Error Handling

* Missing parameters → `400 Bad Request`
* API or server errors → `500 Internal Server Error`
* Full error logs printed to console for debugging

---

## Security Notes

* **Never commit your `.env` file**
* Add `.env` to `.gitignore`
* Rotate API keys if leaked

---

## Future Improvements

* Message history / memory
* Multiple character profiles
* Rate limiting
* Authentication
* Frontend (React / React Native)
* Voice support

---

## License

This project is open-source and free to use for learning and personal projects.

---

If you want, I can also:

* Add **character emotions**
* Add **conversation memory**
* Convert this into a **full chatbot platform**
* Create a **React or React Native frontend**
* Improve AI personality realism

Just tell me.
