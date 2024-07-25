#include <WiFi.h>
#include <esp_now.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Maximum number of peers
#define MAX_PEERS 30
#define MAX_DATA_SIZE 6000
#define CHUNK_SIZE 250
#define DATA_SIZE (CHUNK_SIZE - 6)

const char* ssid = "ESP32_Network";
const char* password = "password123";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Structure to store received data
typedef struct {
    uint8_t mac_addr[6];
    uint8_t* data;
    uint16_t length;
    uint16_t received_length;
    bool sent;
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
    Serial.print("Received chunk from: ");
    printMacAddress(mac_addr);
    Serial.println();

    if (len < 6) {  // MAC address (6 bytes) + chunk data (0 bytes or more)
        Serial.println("Invalid chunk length");
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
        receivedMessages[index].data = (uint8_t*)malloc(MAX_DATA_SIZE * sizeof(uint8_t));
        receivedMessages[index].received_length = 0;
    }

    // Store the MAC address
    memcpy(receivedMessages[index].mac_addr, mac_addr, 6);

    // Append the incoming data chunk to the existing data buffer
    int chunkSize = len - 6;
    if (receivedMessages[index].received_length + chunkSize <= MAX_DATA_SIZE) {
        memcpy(receivedMessages[index].data + receivedMessages[index].received_length, incomingData + 6, chunkSize);
        receivedMessages[index].received_length += chunkSize;
        Serial.print("Chunk received. Total received length: ");
        Serial.println(receivedMessages[index].received_length);
    } else {
        Serial.println("Error: Received data exceeds buffer size.");
    }

    // Print 6 sample integers (the n-thousandth of each array if available)
    Serial.print("Data from ");
    printMacAddress(mac_addr);
    Serial.print(": ");
    for (int i = 0; i < 6 && i * 1000 < receivedMessages[index].received_length; i++) {
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
        if (!receivedMessages[i].sent && receivedMessages[i].received_length == MAX_DATA_SIZE) {
            validIndex = i;
            totalSize = 6 + receivedMessages[i].received_length;  // MAC address + data
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
    memcpy(ptr, receivedMessages[validIndex].data, receivedMessages[validIndex].received_length);

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

    uint8_t myAddress [6];
    WiFi.macAddress(myAddress);
    Serial.print("Mac Address: ");
    for(int i=0; i<6; i++){
        Serial.print(myAddress[i], HEX);
        if (i < 5) {
            Serial.print(":");
        }
    }
    Serial.println();
}

void loop() {
    // Nothing to do here, all logic handled in callbacks and TCP server
}
