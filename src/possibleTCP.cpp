// #include <WiFi.h> //IMPORTANT
// #include <esp_now.h> //IMPORTANT
// // #include <WebServer.h>
// #include "SparkFun_LIS331.h"
// #include <Wire.h> //IMPORTANT
// #include <HardwareSerial.h> //DEBUGGING
// #include <Arduino.h> //IMPORTANT
// #include <cmath> //TEMPORARY
// #include <stdlib.h> //For random number generation

// // Constants
// #define WIFI_CHANNEL 1 //For mesh-network
// #define SEND_INTERVAL 5000 // Send data every 5 seconds
// #define MAX_PEERS 10 // Maximum number of peers to store
// #define AP_SSID "ESP32_AP"
// #define AP_PASSWORD "password"
// #define MAX_MESSAGES 100 // Maximum number of messages to store --> remove eventually
// #define CLEAR_INTERVAL 60000 // Clear messages every 60 seconds --> remove eventually??


// // Variables
// static unsigned int identifier = 0;
// const unsigned int universalIdentifier = 123456789;
// static int SensorValue = 50; //Remove for final, keep for sensor-less
// static int seed = 0; //Remove for final, keep for sensor-less
// static unsigned long last_send_time = 0;
// static unsigned long hi_send_time = 0;
// static unsigned long last_clear_time = 0;

// uint16_t sensor_buffer[10] = {0}; //Used to gather most intense impact for each 100th of a second, preventing double-counting
// uint16_t sensor_data [500] = {0};

// // static float sensor_buffer[5] = {0};
// // static float sensor_data[1000] = {0};

// // static char mes[6] = {0};
// static uint8_t peer_macs[MAX_PEERS][6]; // Array to store MAC addresses of peers
// static int peer_count = 0; // Number of stored peers
// static bool ap_set = false; // Flag to indicate if AP is set
// static bool checked_lowest = false; // Flag to indicate if the lowest MAC check has been done

// // Store received ESP-NOW messages for web server
// String esp_now_messages[MAX_MESSAGES]; //remove after string implimentation
// int message_index = 0; //important

// LIS331 xl;


// //Function to store macAddress as a single uint64
// uint64_t macToUint64(const uint8_t* mac) {
//     uint64_t result = 0;
//     for (int i = 0; i < 6; ++i) {
//         result |= ((uint64_t)mac[i] << (8 * (5 - i)));
//     }
//     return result;
// }


// // Structure to hold the message data
// typedef struct __attribute__((packed)) esp_now_msg_t {
//     uint8_t mac_addr[6];
//     float sensor_data_sending[1000];
// } esp_now_msg_t;

// typedef struct __attribute__((packed)) {
//     uint8_t mac_addr[6];
//     char mes[6];
// } hello_msg_t;

// typedef struct __attribute__((packed)) {
//     uint8_t mac_addr[6];
//     char mes[3];
// } hi_msg_t;

// typedef struct __attribute__((packed)) {
//     uint8_t mac_addr[6];
//     char ssid[32];
// } ap_msg_t;

// // Broadcast MAC address
// static uint8_t broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// // Callback when a message is received
// void msg_recv_callback(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
//     String message = "";
//     if (data_len == sizeof(esp_now_msg_t)) {
//         esp_now_msg_t msg;
//         memcpy(&msg, data, sizeof(esp_now_msg_t));

//         message += "Received sensor data from MAC: ";
//         for (int i = 0; i < 6; i++) {
//             if (msg.mac_addr[i] < 16) message += "0"; // Add leading zero
//             message += String(msg.mac_addr[i], HEX);
//             if (i < 5) message += ":";
//         }
//         message += "\nSensor data: "; //Modify
//         for (int i = 0; i < 5; i++) {
//             message += String(msg.sensor_data_sending[i], 2) + " ";
//         }
//         message += "\n";
//         Serial.print(message);

//     } else if (data_len == sizeof(hello_msg_t)) {
//         hello_msg_t mesg;
//         memcpy(&mesg, data, sizeof(hello_msg_t));

//         message += "Received hello message from MAC: ";
//         for (int i = 0; i < 6; i++) {
//             if (mesg.mac_addr[i] < 16) message += "0"; // Add leading zero
//             message += String(mesg.mac_addr[i], HEX);
//             if (i < 5) message += ":";
//         }
//         message += "\nMessage Received: ";
//         message += mesg.mes;
//         message += "\n";
//         Serial.print(message);

//         // Store the MAC address of the sender
//         if (peer_count < MAX_PEERS) {
//             memcpy(peer_macs[peer_count], mesg.mac_addr, 6);
//             peer_count++;
//             Serial.println("Stored peer MAC address.");
//         } else {
//             Serial.println("Peer list full, cannot store more addresses.");
//         }

//         // Send a hi message in response to the hello message
//         hi_msg_t hi_message;
//         WiFi.macAddress(hi_message.mac_addr);
//         strcpy(hi_message.mes, "Hi");

//         esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)&hi_message, sizeof(hi_msg_t));
//         if (result == ESP_OK) {
//             Serial.print("Sent a hi message to broadcast address: ");
//             for (int i = 0;  i < 6; i++) {
//                 if (broadcast_mac[i] < 16) Serial.print("0"); // Add leading zero
//                 Serial.printf("%02X", broadcast_mac[i]);
//                 if (i < 5) Serial.print(":");
//             }
//             Serial.println();
//             Serial.printf("Message: %s\n", hi_message.mes);
//             hi_send_time = millis(); // Record the time the "Hi" message was sent
//         } else {
//             Serial.print("Failed to send hi message to broadcast address");
//             Serial.printf(", Error: %d\n", result);
//         }
//     } else if (data_len == sizeof(hi_msg_t)) {
//         hi_msg_t mesg;
//         memcpy(&mesg, data, sizeof(hi_msg_t));

//         message += "Received hi message from MAC: ";
//         for (int i = 0; i < 6; i++) {
//             if (mesg.mac_addr[i] < 16) message += "0"; // Add leading zero
//             message += String(mesg.mac_addr[i], HEX);
//             if (i < 5) message += ":";
//         }
//         message += "\nMessage Received: ";
//         message += mesg.mes;
//         message += "\n";
//         Serial.print(message);
//     } else if (data_len == sizeof(ap_msg_t)) {
//         ap_msg_t mesg;
//         memcpy(&mesg, data, sizeof(ap_msg_t));

//         message += "Received AP message from MAC: ";
//         for (int i = 0; i < 6; i++) {
//             if (mesg.mac_addr[i] < 16) message += "0"; // Add leading zero
//             message += String(mesg.mac_addr[i], HEX);
//             if (i < 5) message += ":";
//         }
//         message += "\nAP SSID: ";
//         message += mesg.ssid;
//         message += "\n";
//         Serial.print(message);
//     }

//     // Store the message in the circular buffer
//     esp_now_messages[message_index] = message;
//     message_index = (message_index + 1) % MAX_MESSAGES;
// }

// // Callback when a message is sent
// void msg_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
//     Serial.print("Send status: ");
//     if (status == ESP_NOW_SEND_SUCCESS) {
//         Serial.println("Success");
//     } else {
//         Serial.println("Failure");
//     }
// }

// // Function to send a sensor data message
// void send_msg() {
//     esp_now_msg_t msg;
//     WiFi.macAddress(msg.mac_addr);
//     memcpy(msg.sensor_data_sending, sensor_data, sizeof(sensor_data));

//     esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)&msg, sizeof(esp_now_msg_t));
//     if (result != ESP_OK) {
//         Serial.println("Error sending the message");
//     }

//     // Also store the sensor data message locally
//     String message = "Sent sensor data from MAC: ";
//     for (int i = 0; i < 6; i++) {
//         if (msg.mac_addr[i] < 16) message += "0"; // Add leading zero
//         message += String(msg.mac_addr[i], HEX);
//         if (i < 5) message += ":";
//     }
//     message += "\nSensor data: ";
//     for (int i = 0; i < 5; i++) {
//         message += String(msg.sensor_data_sending[i], 2) + " ";
//     }
//     message += "\n";

//     esp_now_messages[message_index] = message;
//     message_index = (message_index + 1) % MAX_MESSAGES;
// }

// // Function to send a hello message
// void send_hello() {
//     hello_msg_t message;
//     WiFi.macAddress(message.mac_addr);
//     strcpy(message.mes, "Hello");

//     esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)&message, sizeof(hello_msg_t));
//     if (result == ESP_OK) {
//         Serial.print("Sent a hello message to broadcast address: ");
//         for (int i = 0; i < 6; i++) {
//             if (broadcast_mac[i] < 16) Serial.print("0"); // Add leading zero
//             Serial.printf("%02X", broadcast_mac[i]);
//             if (i < 5) Serial.print(":");
//         }
//         Serial.println();
//         Serial.printf("Message: %s\n", message.mes);
//     } else {
//         Serial.print("Failed to send hello message to broadcast address");
//         Serial.printf(", Error: %d\n", result);
//     }
// }

// // Function to send an AP message
// void send_ap_msg() {
//     ap_msg_t message;
//     WiFi.macAddress(message.mac_addr);
//     strcpy(message.ssid, AP_SSID);

//     esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)&message, sizeof(ap_msg_t));
//     if (result == ESP_OK) {
//         Serial.print("Sent an AP message to broadcast address: ");
//         for (int i = 0; i < 6; i++) {
//             if (broadcast_mac[i] < 16) Serial.print("0"); // Add leading zero
//             Serial.printf("%02X", broadcast_mac[i]);
//             if (i < 5) Serial.print(":");
//         }
//         Serial.println();
//         Serial.printf("AP SSID: %s\n", message.ssid);
//     } else {
//         Serial.print("Failed to send AP message to broadcast address");
//         Serial.printf(", Error: %d\n", result);
//     }
// }

// // Function to set up a device as a Wi-Fi Access Point
// void setup_ap() {
//     Serial.println("Setting up as Access Point...");

//     // Initialize Wi-Fi as AP
//     WiFi.softAP(AP_SSID, AP_PASSWORD); // Set SSID and password

//     Serial.println("Access Point setup complete.");

//     // Send AP message
//     send_ap_msg();
// }

// // Web server handle function for root
// // void handleRoot() {
// //     String html = "<html><head><title>ESP32 Data</title>";
// //     html += "<meta http-equiv='refresh' content='5'></head><body>";
// //     html += "<h1>ESP-NOW Data</h1>";
// //     html += "<pre>";
// //     for (int i = 0; i < MAX_MESSAGES; i++) {
// //         int index = (message_index + i) % MAX_MESSAGES;
// //         if (esp_now_messages[index].length() > 0) {
// //             html += esp_now_messages[index];
// //             html += "\n";
// //         }
// //     }
// //     html += "</pre>";
// //     html += "</body></html>";
// //     server.send(200, "text/html", html);
// // }

// void sendToDevice() {
    

// }


// // Setup function
// void setup() {
//     pinMode(9,INPUT);       // Interrupt pin input
//     Wire.begin(25, 26);
    
//     xl.setI2CAddr(0x19);
//     xl.begin(LIS331::USE_I2C);

//     Serial.begin(115200);

//     // Initialize Wi-Fi
//     WiFi.mode(WIFI_AP_STA); // Set Wi-Fi mode to both Station and Access Point
//     WiFi.disconnect();
//     uint8_t mac[6];
//     WiFi.macAddress(mac);
//     identifier = macToUint64(mac);

//     // Initialize ESP-NOW
//     if (esp_now_init() != ESP_OK) {
//         Serial.println("Error initializing ESP-NOW");
//         return;
//     }

//     // Set up the peer information
//     esp_now_peer_info_t peer_info = {};
//     memcpy(peer_info.peer_addr, broadcast_mac, 6);
//     peer_info.channel = WIFI_CHANNEL;
//     peer_info.encrypt = false;

//     if (esp_now_add_peer(&peer_info) != ESP_OK) {
//         Serial.println("Failed to add peer");
//         return;
//     }

//     // Register the callbacks
//     esp_now_register_recv_cb(msg_recv_callback);
//     esp_now_register_send_cb(msg_send_cb);

//     // Set up web server
//     // server.on("/", handleRoot);
//     // server.begin();
//     // Serial.println("Web server started.");

//     delay(5000);

//     // Send hello message
//     send_hello();
// }

// void loop() {
//   int16_t x, y, z;
//     // Update sensor data (dummy data for now)
//     for (int i = 0; i < 5; i++) {
//         xl.readAxes(x, y, z);
//         float a_x = xl.convertToG(100,x);
//         float a_y = xl.convertToG(100,x);
//         float a_z = xl.convertToG(100,x);

//         float a_absolute = sqrt(a_x*a_x+a_y*a_y+a_z*a_z);
//         sensor_data[i] = a_absolute;


//         // int seed = random(0, 10); // Generating a seed between 0 and 10
//         // if (SensorValue > 0) {
//         //     sensor_data[i] = SensorValue + seed;
//         // } else {
//         //     sensor_data[i] = SensorValue + random(seed * -1, seed);
//         // }
//     }

//     // Serial.print("X Acceleration (g):  ");
//     // Serial.println(xl.convertToG(100,x));
//     // Send sensor data every SEND_INTERVAL milliseconds
//     if (millis() - last_send_time >= SEND_INTERVAL) {
//         last_send_time = millis();
//         send_msg();
//     }

//     // Check if it's time to set up as Access Point
//     if (!ap_set && !checked_lowest && millis() - hi_send_time >= 15000 && hi_send_time != 0) {
//         // Find the device with the lowest MAC address
//         uint8_t my_mac[6];
//         WiFi.macAddress(my_mac);

//         Serial.print("Checking if this device has the lowest MAC address... My MAC: ");
//         for (int i = 0; i < 6; i++) {
//             if (my_mac[i] < 16) Serial.print("0"); // Add leading zero
//             Serial.printf("%02X", my_mac[i]);
//             if (i < 5) Serial.print(":");
//         }
//         Serial.println();

//         bool is_lowest = true;
//         for (int i = 0; i < peer_count; i++) {
//             Serial.print("Comparing with peer MAC: ");
//             for (int j = 0; j < 6; j++) {
//                 if (peer_macs[i][j] < 16) Serial.print("0"); // Add leading zero
//                 Serial.printf("%02X", peer_macs[i][j]);
//                 if (j < 5) Serial.print(":");
//             }
//             Serial.println();

//             if (memcmp(my_mac, peer_macs[i], 6) > 0) {
//                 is_lowest = false;
//                 break;
//             }
//         }

//         if (is_lowest) {
//             Serial.println("This device has the lowest MAC address. Setting up as Access Point...");
//             setup_ap();
//             ap_set = true;
//         } else {
//             Serial.println("This device does not have the lowest MAC address.");
//         }
//         checked_lowest = true; // Ensure this check is done only once
//     }

//     // Handle web server
//     // server.handleClient();

//     // Clear messages periodically to free up memory
//     if (millis() - last_clear_time >= CLEAR_INTERVAL) {
//         last_clear_time = millis();
//         for (int i = 0; i < MAX_MESSAGES; i++) {
//             esp_now_messages[i] = "";
//         }
//         message_index = 0;
//         Serial.println("Cleared stored messages.");
//     }
// }
