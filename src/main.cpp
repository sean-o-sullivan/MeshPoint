#include <WiFi.h>
#include <esp_now.h>
#include <stdlib.h>

#define CHUNK_SIZE 250
#define DATA_SIZE (CHUNK_SIZE - 6) // 250 bytes total minus 6 bytes for MAC address

// Broadcast MAC address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure to define the message
typedef struct struct_message {
    uint8_t mac_addr[6];
    uint8_t data[DATA_SIZE];
} struct_message;

// Create a struct_message to hold the data
struct_message myData;

// Function to generate uint8_t's
void generateRandomData(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        float rand_val = (float)rand() / RAND_MAX;
        if (rand_val < 0.99) {
            data[i] = rand() % 4; // 0 to 3
        } else {
            data[i] = 10 + rand() % 141; // 10 to 150
        }
    }
}

// Callback when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Print MAC address of the transmitter
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("Transmitter MAC Address: ");
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 16) {
            Serial.print("0");
        }
        Serial.print(mac[i], HEX);
        if (i < 5) {
            Serial.print(":");
        }
    }
    Serial.println();

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
    esp_now_register_send_cb(onDataSent);

    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Set up the MAC address of this device
    WiFi.macAddress(myData.mac_addr);
}

void loop() {
    // Generate random data
    uint8_t fullData[6000];
    generateRandomData(fullData, 6000);

    // Send the data in chunks of 250 bytes, including the MAC address
    for (size_t i = 0; i < 6000; i += DATA_SIZE) {
        size_t chunkSize = (i + DATA_SIZE <= 6000) ? DATA_SIZE : (6000 - i);
        memcpy(myData.data, fullData + i, chunkSize);
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, 6 + chunkSize);

        if (result == ESP_OK) {
            Serial.print("Chunk ");
            Serial.print(i / DATA_SIZE);
            Serial.println(" sent with success");
        } else {
            Serial.print("Chunk ");
            Serial.print(i / DATA_SIZE);
            Serial.print(" failed to send, error code: ");
            Serial.println(result);
        }

        // Increase delay between sending chunks to avoid congestion
        delay(100);  // Increase delay to give more time for processing
    }

    // Wait for a random interval between 5 and 10 seconds
    int waitTime = 5000 + rand() % 5001; // 5000 to 10000 milliseconds
    delay(waitTime);
}
 