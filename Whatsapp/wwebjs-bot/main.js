const express = require('express');
const { Client, LocalAuth, MessageMedia } = require('whatsapp-web.js');
const qrcode = require('qrcode-terminal');

const app = express();
app.use(express.json());

const client = new Client({
    authStrategy: new LocalAuth({ clientId: 'main-session' }),
    puppeteer: { headless: true }
});

client.on('qr', qr => {
    qrcode.generate(qr, { small: true });
    console.log('Scan QR');
});

client.on('ready', () => {
    console.log('WhatsApp client ready');
});

client.initialize();

/* ======================================
   SINGLE POST ROUTE (TEXT / IMAGE / LINK)
   ====================================== */
app.post('/send', async (req, res) => {
    try {
        const { number, message, imageUrl, caption } = req.body;

        if (!number) {
            return res.status(400).json({ error: 'number is required' });
        }

        const chatId = number + '@c.us';

        // IMAGE
        if (imageUrl) {
            const media = await MessageMedia.fromUrl(imageUrl);
            await client.sendMessage(chatId, media, { caption });
            return res.json({ status: 'sent', type: 'image' });
        }

        // TEXT or LINK
        if (message) {
            await client.sendMessage(chatId, message);
            return res.json({ status: 'sent', type: 'text/link' });
        }

        return res.status(400).json({
            error: 'Provide message or imageUrl'
        });

    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

app.listen(3000, () => {
    console.log('API running at http://localhost:3000');
});
