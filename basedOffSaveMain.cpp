#include <WiFi.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <Arduino.h>


// Constants
#define WIFI_CHANNEL 1
#define SEND_INTERVAL 1000 // Send data every 5 seconds
#define MAX_PEERS 10 // Maximum number of peers to store
#define AP_SSID "ESP32_AP"
#define AP_PASSWORD "password"
#define MAX_MESSAGES 2500 // Maximum number of messages to store
#define CLEAR_INTERVAL 5000 // Clear messages every 60 seconds
#define WINDOW_SIZE 50
#define SENSOR_BUFFER_SIZE 100

// Variables
static int SensorValue = 50;
static int seed = 0;
static int timeSinceLastMessage = 0;
static int timeAtLastMessage = 0;

static unsigned long last_send_time = 0;
static unsigned long hi_send_time = 0;
static unsigned long last_clear_time = 0;
static float sensor_buffer[SENSOR_BUFFER_SIZE] = {0};
static int bufferCount = 0;
static float sensor_data[WINDOW_SIZE] = {0};
static int arrayCount =0;
static char mes[6] = {0};
static uint8_t peer_macs[MAX_PEERS][6]; // Array to store MAC addresses of peers
static int peer_count = 0; // Number of stored peers
static bool ap_set = false; // Flag to indicate if AP is set
static bool checked_lowest = false; // Flag to indicate if the lowest MAC check has been done

// Store received ESP-NOW messages for web server
String esp_now_messages[MAX_MESSAGES];
int message_index = 0;






// Structure to hold the message data



typedef struct __attribute__((packed)) {
    uint8_t mac_addr[6];
    char mes[10];
} hello_msg_t;


// Broadcast MAC address
static uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void msg_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    String message = "";
    if (data_len == sizeof(hello_msg_t)) {
        timeAtLastMessage = millis();
        hello_msg_t mesg;
        memcpy(&mesg, data, sizeof(hello_msg_t));

        Serial.print("Gotten a message");

        message += "Still connected to: ";
        for (int i = 0; i < 6; i++) {
            if (mesg.mac_addr[i] < 16) message += "0"; // Add leading zero
            message += String(mesg.mac_addr[i], HEX);
            if (i < 5) message += ":";
        }
        message += "\nMessage Received: ";
        message += mesg.mes;
        message += "\n";
        Serial.print(message);

        // Store the MAC address of the sender
        if (peer_count < MAX_PEERS) {
            memcpy(peer_macs[peer_count], mesg.mac_addr, 6);
            peer_count++;
            Serial.println("Stored peer MAC address.");
        } else {
            // Serial.println("Peer list full, cannot store more addresses.");
        }

    // Store the message in the circular buffer
    esp_now_messages[message_index] = message;
    message_index = (message_index + 1) % MAX_MESSAGES;
}
}


// Function to send a hello message
void send_hello() {
    // Serial.println("Should be sending");
    hello_msg_t message;
    WiFi.macAddress(message.mac_addr);
    strcpy(message.mes, "Connected");

    esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)&message, sizeof(hello_msg_t));
    // if (result == ESP_OK) {
    //     // Serial.print("Still connected to: ");
    //     // for (int i = 0; i < 6; i++) {
    //     //     if (broadcast_mac[i] < 16) Serial.print("0"); // Add leading zero
    //     //     Serial.printf("%02X", broadcast_mac[i]);
    //     //     if (i < 5) Serial.print(":");
    //     }
    //     Serial.println();
    //     // Serial.printf("Message: %s\n", message.mes);
    // } else {
    //     Serial.print("No longer connected \n");
    //     Serial.printf(", Error: %d\n", result);
    // }
}


void setup() {

    Serial.begin(115200);

    // Initialize Wi-Fi
    WiFi.mode(WIFI_AP_STA); // Set Wi-Fi mode to both Station and Access Point
    WiFi.disconnect();

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Set up the peer information
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, broadcast_mac, 6);
    peer_info.channel = WIFI_CHANNEL;
    peer_info.encrypt = false;

    if (esp_now_add_peer(&peer_info) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Register the callbacks
    esp_now_register_recv_cb(msg_recv_cb);
    // esp_now_register_send_cb(msg_send_cb);

    // // Set up web server
    // server.on("/", handleRoot);
    // server.begin();
    // Serial.println("Web server started.");

    delay(5000);

    // Send hello message
    send_hello();
}

void loop() {



    if (millis() - last_send_time >= SEND_INTERVAL) {
        last_send_time = millis();
        send_hello();
        timeSinceLastMessage = millis()-timeAtLastMessage;
        if(timeSinceLastMessage>3000){
            Serial.print("Disconnected for ");
            Serial.print(timeSinceLastMessage);
            Serial.println("miliseconds");
        }
    }

   


    // Clear messages periodically to free up memory
    if (millis() - last_clear_time >= CLEAR_INTERVAL) {
        last_clear_time = millis();
        arrayCount = 0;
        for(int i=0; i<WINDOW_SIZE; i++){sensor_data[i] = 0;}
        for (int i = 0; i < MAX_MESSAGES; i++) {
            esp_now_messages[i] = "";
        }
        message_index = 0;
        // Serial.println("Cleared stored messages.");
    }
}

