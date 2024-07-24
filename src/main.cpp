#include <WiFi.h>
#include <esp_now.h>
#include <stdlib.h>

// Define MAC address of the receiver (replace with the actual MAC address)
uint8_t receiverMAC[] = {0x24, 0x6F, 0x28, 0xA1, 0xB2, 0xC3};

// Structure to define the message
typedef struct struct_message {
    uint8_t mac_addr[6];
    uint8_t data[6000];
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

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Register the send callback
    esp_now_register_send_cb(onDataSent);

    // Register peer
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiverMAC, 6);
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
    generateRandomData(myData.data, 6000);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(receiverMAC, (uint8_t *) &myData, sizeof(myData));

    if (result == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }

    // Wait for a random interval between 5 and 10 seconds
    int waitTime = 5000 + rand() % 5001; // 5000 to 10000 milliseconds
    delay(waitTime);
}
