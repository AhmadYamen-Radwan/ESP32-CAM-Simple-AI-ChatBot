#include <Arduino.h>
#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ===================
// Credentials
// ===================
const char* ssid = "Network SSID";
const char* password = "Network Passowrd";
const char* bot_token = "Telegram Bot Token";
const char* ollama_token = "OLLAMA API KEY";
const char* ollama_model = "MODEL NAME";

// Shared secure client for Telegram
WiFiClientSecure secure;
UniversalTelegramBot bot(bot_token, secure);

unsigned long lastTimeBotRan;
int botRequestDelay = 1000;


const int MAX_QUEUE = 1;
String pendingChatIds[MAX_QUEUE];
String pendingMessages[MAX_QUEUE];
int queueHead = 0;
int queueTail = 0;
int queueCount = 0;


// Send prompt to Ollama via /api/chat
String askOllama(String prompt) 
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, "https://ollama.com/api/chat");
    http.setReuse(false);

    http.addHeader("Content-Type", "application/json");
    String authHeader = "Bearer " + String(ollama_token);
    http.addHeader("Authorization", authHeader);

    // Build request
    StaticJsonDocument<512> doc;
    doc["model"] = ollama_model;
    doc["stream"] = false;
    doc["think"] = false;

    JsonArray messages = doc.createNestedArray("messages");
    JsonObject userMsg = messages.createNestedObject();
    userMsg["role"] = "user";
    userMsg["content"] = prompt;

    String requestBody;
    serializeJson(doc, requestBody);

    // First attempt
    int httpCode = http.POST(requestBody);

    // Retry once on connection failure
    if (httpCode != HTTP_CODE_OK) 
    {
        http.end();
        delay(2000);
        
        http.begin(client, "https://ollama.com/api/chat");
        http.setReuse(false);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", authHeader);
        httpCode = http.POST(requestBody);
    }

    if (httpCode != HTTP_CODE_OK) 
    {
        http.end();
        return "Error: HTTP " + String(httpCode);
    }

    String response = http.getString();
    http.end();

    DynamicJsonDocument respDoc(2048);
    DeserializationError err = deserializeJson(respDoc, response);

    if (err) 
    {
        return "Error: JSON parse failed";
    }

    if (respDoc.containsKey("message")) 
    {
        JsonObject msg = respDoc["message"];
        String content = msg["content"].as<String>();
        if (content.length() > 0) 
        {
            return content;
        }
        if (msg.containsKey("thinking")) 
        {
            String thinking = msg["thinking"].as<String>();
            if (thinking.length() > 0) 
            {
                return thinking;
            }
        }
    }

    return "Error: No content in response";
}

// Process next queued request
void processNextInQueue() 
{
    if (queueCount == 0) return;

    String chat_id = pendingChatIds[queueHead];
    String msg = pendingMessages[queueHead];

    queueHead = (queueHead + 1) % MAX_QUEUE;
    queueCount--;

    String reply = askOllama(msg);

    if (reply == "null" || reply.startsWith("Error:")) 
    {
        reply = "Sorry, couldn't process that. Try again with different context.";
    }

    bot.sendMessage(chat_id, reply, "");
}


// Handle incoming Telegram messages
void handleNewMessages(int numNewMessages) 
{
    for (int i = 0; i < numNewMessages; i++) 
    {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;

        if (text == "/start") 
        {
            bot.sendMessage(chat_id, "Send me a message and I'll ask Ollama.", "");
            continue;
        }

        if (queueCount >= MAX_QUEUE) 
        {
            bot.sendMessage(chat_id, "Server busy, please try again later.", "");
            continue;
        }

        pendingChatIds[queueTail] = chat_id;
        pendingMessages[queueTail] = text;
        queueTail = (queueTail + 1) % MAX_QUEUE;
        queueCount++;

        if (queueCount == 1) 
        {
            processNextInQueue();
        } 
        else 
        {
            bot.sendMessage(chat_id, "Queued, position: " + String(queueCount), "");
        }
    }
}


// Setup
void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
    }

    secure.setInsecure();
}

// Main Loop
void loop() {
    if (millis() - lastTimeBotRan > botRequestDelay) 
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        if (numNewMessages > 0) 
        {
            handleNewMessages(numNewMessages);
        }
        lastTimeBotRan = millis();
    }

    if (queueCount > 0) {
        processNextInQueue();
    }
}