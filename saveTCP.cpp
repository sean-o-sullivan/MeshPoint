#include <WiFi.h>
#include <esp_now.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Maximum number of peers
#define MAX_PEERS 30

const char* ssid = "ESP32_Network";
const char* password = "password123";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Structure to store received data
typedef struct {
    uint8_t mac_addr[6];
    uint8_t* data;
    uint16_t length; // Add length to handle varying data sizes
    bool sent;       // Flag to indicate if the data has been sent
} BroadcastMessage;

// Array to store received messages
BroadcastMessage receivedMessages[MAX_PEERS];

// Index to keep track of stored messages
int currentIndex = 0;

// TCP server on port 80
AsyncWebServer server(80);

// Function to print MAC address to serial
void printMacAddress(const uint8_t* mac_addr) {
    for (int i = 0; i < 6; i++) {
        if (mac_addr[i] < 16) {
            Serial.print("0");
        }
        Serial.print(mac_addr[i], HEX);
        if (i < 5) {
            Serial.print(":");
        }
    }
}

// Callback when data is received
void onDataRecv(const uint8_t* mac_addr, const uint8_t* incomingData, int len) {
    Serial.print("Received message from: ");
    printMacAddress(mac_addr);
    Serial.println();

    if (len != 6006) {  // 6 bytes MAC address + 6000 bytes data
        Serial.println("Invalid message length");
        return;
    }

    // Check if MAC address already exists
    int index = -1;
    for (int i = 0; i < currentIndex; i++) {
        if (memcmp(receivedMessages[i].mac_addr, mac_addr, 6) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        // New MAC address, store in the next available slot
        if (currentIndex < MAX_PEERS) {
            index = currentIndex++;
        } else {
            // If we've reached the max number of peers, override the oldest entry
            index = 0;
            currentIndex = 1;
        }
        receivedMessages[index].data = (uint8_t*)malloc(6000 * sizeof(uint8_t));
    }

    // Store the MAC address and data
    memcpy(receivedMessages[index].mac_addr, mac_addr, 6);
    memcpy(receivedMessages[index].data, incomingData + 6, 6000);
    receivedMessages[index].length = 6000;
    receivedMessages[index].sent = false;  // Mark as new data

    // Print 6 sample integers (the n-thousandth of each array)
    Serial.print("Data from ");
    printMacAddress(mac_addr);
    Serial.print(": ");
    for (int i = 0; i < 6; i++) {
        Serial.print(receivedMessages[index].data[i * 1000]);
        if (i < 5) {
            Serial.print(", ");
        }
    }
    Serial.println();
}

// Initialize ESP-NOW
void initESPNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(onDataRecv);
}

// TCP request handler
void onRequest(AsyncWebServerRequest* request) {
    size_t totalSize = 0;
    int validIndex = -1;
    for (int i = 0; i < currentIndex; i++) {
        if (!receivedMessages[i].sent) {
            validIndex = i;
            totalSize = 6 + 2 + receivedMessages[i].length;  // MAC address + length prefix + data
            break;
        }
    }

    if (validIndex == -1) {
        request->send(200, "text/plain", "No new data available.");
        return;
    }

    uint8_t* responseBuffer = (uint8_t*)malloc(totalSize);
    uint8_t* ptr = responseBuffer;
    memcpy(ptr, receivedMessages[validIndex].mac_addr, 6);
    ptr += 6;
    ptr[0] = (uint8_t)((receivedMessages[validIndex].length >> 8) & 0xFF); // High byte of length
    ptr[1] = (uint8_t)(receivedMessages[validIndex].length & 0xFF);        // Low byte of length
    ptr += 2;
    memcpy(ptr, receivedMessages[validIndex].data, receivedMessages[validIndex].length);

    // Create a response to send the binary data
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/octet-stream", responseBuffer, totalSize);
    response->addHeader("Content-Disposition", "attachment; filename=data.bin");
    request->send(response);
    free(responseBuffer);

    // Mark the data as sent
    receivedMessages[validIndex].sent = true;
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Configure static IP address
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        Serial.println("AP Config Failed");
    }

    // Initialize Wi-Fi as Station and SoftAP
    WiFi.softAP(ssid, password);

    // Initialize ESP-NOW
    initESPNow();

    // Initialize TCP server
    server.on("/", HTTP_GET, onRequest);
    server.begin();

    Serial.println("ESP32 AP is running");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    // Nothing to do here, all logic handled in callbacks and TCP server
}
