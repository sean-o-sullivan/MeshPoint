#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <Arduino.h>

// Constants
#define WIFI_CHANNEL 1
#define SEND_INTERVAL 1000 // Send data every 1 second

// Structure to hold the message data
typedef struct __attribute__((packed)) {
    char message[32];
} broadcast_message_t;

// Broadcast MAC address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Function to send a broadcast message
void send_broadcast_message() {
    broadcast_message_t message;
    strcpy(message.message, "Hello from ESP32");

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&message, sizeof(message));
    if (result == ESP_OK) {
        Serial.println("Broadcast message sent successfully");
    } else {
        Serial.print("Error sending broadcast message: ");
        Serial.println(result);
    }
}

// Callback function to handle received messages
void on_message_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    char message[data_len + 1];
    memcpy(message, data, data_len);
    message[data_len] = '\0'; // Null-terminate the string

    Serial.print("Received message from: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_addr[i]);
        if (i < 5) {
            Serial.print(":");
        }
    }
    Serial.print(" - ");
    Serial.println(message);
}

void setup() {
    Serial.begin(115200);

    // Initialize Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the callback function for received messages
    esp_now_register_recv_cb(on_message_recv);

    // Add the broadcast peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add broadcast peer");
        return;
    }

    Serial.println("ESP-NOW Initialized and Broadcast Peer Added");
}

void loop() {
    static unsigned long last_send_time = 0;
    if (millis() - last_send_time >= SEND_INTERVAL) {
        last_send_time = millis();
        send_broadcast_message();
    }
}
