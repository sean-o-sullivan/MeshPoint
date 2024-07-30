#include <painlessMesh.h>
#include <esp_now.h>
#include <WiFi.h>
#include <unordered_map>

#define MESH_SSID "ForceMesh"
#define MESH_PORT 5555
#define DATA_SIZE 5994  // 6000 - 6
#define SCAN_TIME 10000
#define SETUP_COMMAND 2069783108202043734ULL

painlessMesh mesh;
bool scanMode = false;
unsigned long scanEndTime = 0;
std::unordered_map<uint32_t, unsigned long> receivedIdentifiers;
uint8_t routerAddress[6];
unsigned long lastScanAttempt = 0;
unsigned long lastDataSend = 0;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(onDataRecv);

    // Start in scan mode
    startScanMode();
}

void loop() {
    unsigned long currentMillis = millis();

    // Cleanup old entries
    for (auto it = receivedIdentifiers.begin(); it != receivedIdentifiers.end(); ) {
        if (currentMillis - it->second > SCAN_TIME) {
            it = receivedIdentifiers.erase(it);
        } else {
            ++it;
        }
    }

    if (scanMode && currentMillis > scanEndTime) {
        scanMode = false;
        Serial.println("Scan mode ended.");
        initMesh();
    }

    if (!scanMode) {
        mesh.update();

        // Generate and send random data every minute
        if (currentMillis - lastDataSend >= 60000) {
            sendRandomData();
            lastDataSend = currentMillis;
        }

        // Send router address via ESP-NOW
        if (currentMillis - lastScanAttempt >= 5000) {
            esp_now_send(routerAddress, (uint8_t*)"ping", 4);
            lastScanAttempt = currentMillis;
        }
    }
}

void startScanMode() {
    Serial.println("Starting scan mode...");
    scanMode = true;
    scanEndTime = millis() + SCAN_TIME;
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.channel(1);  // You might need to scan multiple channels
}

void initMesh() {
    String password = String(SETUP_COMMAND);
    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_SSID, password.c_str(), MESH_PORT, WIFI_AP_STA);
    mesh.onReceive(&receivedCallback);
}

void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    if (scanMode && len == sizeof(SETUP_COMMAND)) {
        uint64_t receivedCommand;
        memcpy(&receivedCommand, data, sizeof(SETUP_COMMAND));
        if (receivedCommand == SETUP_COMMAND) {
            memcpy(routerAddress, mac_addr, 6);
            Serial.println("Setup command received, router address saved");
            scanMode = false;
            initMesh();
        }
    }
}

void receivedCallback(uint32_t from, String &msg) {
    int firstComma = msg.indexOf(',');
    int secondComma = msg.indexOf(',', firstComma + 1);
    
    String macStr = msg.substring(0, firstComma);
    uint32_t messageTag = (uint32_t) strtol(msg.substring(firstComma + 1, secondComma).c_str(), NULL, 10);
    unsigned long currentMillis = millis();

    // Check for duplicate message within the last 10 seconds
    if (receivedIdentifiers.find(messageTag) != receivedIdentifiers.end() && 
        (currentMillis - receivedIdentifiers[messageTag] < SCAN_TIME)) {
        Serial.println("Duplicate message received within the last 10 seconds, ignoring.");
        return;
    }

    // Update the received message time
    receivedIdentifiers[messageTag] = currentMillis;

    // Forward the message to other nodes
    mesh.sendBroadcast(msg);

    // You can process the MAC address here if needed
    Serial.print("Received message from MAC: ");
    Serial.println(macStr);
}


void sendRandomData() {
    uint8_t data[DATA_SIZE];
    generateRandomData(data, DATA_SIZE);

    // Get this device's MAC address
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // Add random 32-bit identifier
    uint32_t identifier = esp_random();

    // Construct the message with MAC address, identifier, and data
    String msg = "";
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 0x10) msg += "0";
        msg += String(mac[i], HEX);
    }
    msg += "," + String(identifier) + ",";
    for (int i = 0; i < DATA_SIZE; i++) {
        msg += String(data[i]) + ",";
    }

    // Broadcast to mesh
    mesh.sendBroadcast(msg);

    // Send to router via ESP-NOW
    esp_now_send(routerAddress, (uint8_t*)msg.c_str(), msg.length());
}


void generateRandomData(uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        float rand_val = (float)esp_random() / UINT32_MAX;
        if (rand_val < 0.99) {
            data[i] = esp_random() % 4; // 0 to 3
        } else {
            data[i] = 10 + esp_random() % 141; // 10 to 150
        }
    }
}