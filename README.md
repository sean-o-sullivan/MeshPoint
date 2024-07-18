# ESP32 MeshPoint: Real-Time Data Sharing and Web Interface

**ESP32 MeshPoint** creates a mesh network using ESP-NOW for wireless communication between ESP32 devices, with one device serving as an Access Point (AP) to display real-time data on a web page.

## Features
- Mesh Network: ESP32 devices form a mesh network for data sharing.
- ESP-NOW Communication: Low-power, wireless communication between devices.
- Web Server: Displays received messages and sensor data.
- Dynamic AP Setup: Device with lowest MAC address sets up as AP.
- Real-Time Data: View live data by connecting to AP and navigating to `192.168.4.1`.

## Usage
1. Upload Code: Upload provided code to multiple ESP32 devices.
2. Power Up: Power up devices to start communication.
3. Connect to AP: Connect to AP using specified SSID and password.
4. View Data: Open `192.168.4.1` in web browser to see live data.

This project demonstrates data sharing through a mesh network of ESP32 devices with a web interface for monitoring.
