require('dotenv').config();
const express = require('express');
const axios = require('axios');
const cors = require('cors');

console.log('[INIT] Loading environment variables');

const app = express();
app.use(cors());
app.use(express.json());

console.log('[INIT] Express app initialized');

app.post('/chat', async (req, res) => {
    console.log('[REQUEST] /chat endpoint hit');
    console.log('[REQUEST BODY]', req.body);

    try {
        const { number, message } = req.body;

        if (!number || !message) {
            console.error('[VALIDATION ERROR] number or message missing');
            return res.status(400).json({ error: 'number and message required' });
        }

        console.log('[VALIDATION] Input validated');
        console.log(`[OPENROUTER] Sending message → "${message}"`);

        const ai = await axios.post(
            'https://openrouter.ai/api/v1/chat/completions',
            {
                model: 'mistralai/mistral-7b-instruct',
                messages: [
                    {
                        role: 'system',
                        content: 'You are a friendly female game character.'
                    },
                    {
                        role: 'user',
                        content: message
                    }
                ]
            },
            {
                headers: {
                    Authorization: `Bearer ${process.env.OPENROUTER_API_KEY}`,
                    'Content-Type': 'application/json'
                }
            }
        );

        console.log('[OPENROUTER] Response received');

        const reply = ai.data.choices[0].message.content;
        console.log('[OPENROUTER] AI Reply →', reply);

        console.log('[WHATSAPP] Sending reply to WhatsApp server');

        await axios.post(process.env.WHATSAPP_API, {
            number,
            message: reply
        });

        console.log('[WHATSAPP] Message successfully sent');

        res.json({ status: 'success', reply });

    } catch (err) {
        console.error('[ERROR] Something went wrong');
        console.error('[ERROR MESSAGE]', err.message);

        if (err.response) {
            console.error('[ERROR RESPONSE DATA]', err.response.data);
        }

        res.status(500).json({
            error: 'Internal Server Error',
            details: err.message
        });
    }
});

app.listen(4000, () => {
    console.log('[SERVER] AI Server running at http://localhost:4000');
});
