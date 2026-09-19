# ESP32-CAM Simple AI ChatBot

A simple AI chatbot project for the ESP32-CAM, built with PlatformIO.

## Hardware note

This project was built and tested using a spare ESP32-CAM whose PSRAM and camera sensor are broken. Because of that, camera functionality may not work, and the board may behave unpredictably when handling large memory-heavy requests. This project is intended as a lightweight Telegram + Ollama chatbot test, not a full camera-enabled AI vision platform.

## Large-context limitation

When processing a large context, the device may return `null` or an HTTP `-11` error. This is expected on hardware with limited or faulty memory resources, especially when PSRAM is unavailable or defective. If that happens, reduce the message size or context length and try again.

## 1. Edit `src/main.cpp`

Open `src/main.cpp` and update the credentials near the top:

```cpp
const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";
const char* bot_token = "1234567890:AAExampleTelegramBotToken";
const char* ollama_token = "ollama_xxxxxxxxxxxxxxxxx";
const char* ollama_model = "your-model-name";
```

### Required values

- `ssid`: your Wi-Fi network name (2.4 GHz recommended)
- `password`: your Wi-Fi password
- `bot_token`: Telegram bot token from BotFather
- `ollama_token`: Ollama API key
- `ollama_model`: model name available to your Ollama account

Do not commit real tokens to GitHub. Anyone with the bot token can control the bot, and anyone with the Ollama key can use your account.

## 2. Create and configure the Telegram bot

1. Open Telegram.
2. Search for `@BotFather`.
3. Send:

```text
/newbot
```

4. Enter a name and a username ending with `bot`.
5. BotFather will return a token such as:

```text
1234567890:AAxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
```

6. Copy that token into `bot_token` in `src/main.cpp`.
7. Open the bot and press Start or send `/start`.

The code handles `/start` and replies:

```text
Send me a message and I'll ask Ollama.
```

The bot uses polling, so no webhook setup is required for this project.

## 3. Configure Ollama

This project sends requests to:

```text
https://ollama.com/api/chat
```

You need:

- an Ollama account with cloud API access,
- an Ollama API key,
- a model name available to your account.

Then set:

```cpp
const char* ollama_token = "YOUR_OLLAMA_API_KEY";
const char* ollama_model = "YOUR_MODEL_NAME";
```

If the model name is incorrect, the device may return an HTTP error or empty response.

## 4. Install PlatformIO

The project is built using PlatformIO.

### Required tools

- VS Code
- PlatformIO IDE extension

Install PlatformIO IDE from the VS Code Extensions marketplace and reopen the project folder.

## 5. Upload the firmware

The project already includes `platformio.ini`:

```ini
[env:esp32cam]
platform = espressif32 @ 6.12.0
board = esp32cam
framework = arduino
board_build.flash_mode = dout
monitor_speed = 115200
```

### Wiring for upload

Most ESP32-CAM boards do not have a built-in USB port. You usually need a USB-to-TTL serial adapter such as an FTDI or CP2102.

Connect:

| USB-TTL adapter | ESP32-CAM |
|---|---|
| TX | U0R / RX |
| RX | U0T / TX |
| GND | GND |
| 5V | 5V |

Important: TX and RX must be crossed.

To enter flashing mode:

1. Connect `GPIO0` to `GND`
2. Connect the USB-TTL adapter
3. Press reset if needed
4. Upload from PlatformIO
5. After upload, disconnect `GPIO0` from `GND`
6. Press reset again

### Build and upload

From VS Code with PlatformIO:

1. Open the project folder
2. Click the PlatformIO icon
3. Build the project
4. Upload the firmware

From terminal:

```bash
pio run
pio run --target upload
pio device monitor -b 115200
```

If needed, specify a serial port:

```bash
pio run --target upload --upload-port COM5
```

On Linux/macOS it may be something like:

```bash
/dev/ttyUSB0
```

## 6. What the device does at runtime

After startup, the board:

1. Starts serial output at `115200`
2. Connects to Wi-Fi
3. Polls Telegram for new messages
4. Sends each new message to Ollama
5. Returns the response to the same Telegram chat

The program checks for new messages with:

```cpp
int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
```

## 7. Troubleshooting

### Wi-Fi does not connect

- Use a 2.4 GHz Wi-Fi network
- Confirm SSID and password are correct
- Check power supply stability
- Verify the board is in a good power state

### Telegram bot is not responding

- The bot token may be incorrect
- The bot may not have started in Telegram
- The ESP32 may not be connected to Wi-Fi
- Serial monitor may show connection or upload issues

### Ollama returns `null` or HTTP `-11`

- Try a shorter prompt
- Reduce context length
- Avoid oversized messages
- Check that the model name is valid
- Check the Ollama API key

### Camera not working

This project is not expected to have working camera functionality because the board in use has a broken PSRAM and broken camera sensor.

## 8. Notes

The code uses `setInsecure()` for HTTPS connections to simplify testing:

```cpp
secure.setInsecure();
client.setInsecure();
```

This is acceptable for a personal test project, but not ideal for production. For production use, certificate validation should be added.

## 9. Project status

This repository is a simple ESP32-CAM Telegram chatbot prototype using PlatformIO and Ollama. It is best suited for testing on hardware that has functioning Wi-Fi and memory. On boards with broken PSRAM or camera hardware, camera features and larger requests may not work properly.
