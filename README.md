# ESP32 MeshPoint: Real-Time Data Sharing and Web Display

![ESP32 MeshPoint Project](path_to_your_image.png)

**ESP32 MeshPoint** is a project that creates a mesh network using ESP-NOW for wireless communication between ESP32 devices, with one device serving as an Access Point (AP) to display real-time data on a web page.

## Features

- **Mesh Network:** ESP32 devices form a mesh network for efficient data sharing.
- **ESP-NOW Communication:** Low-power, wireless communication between devices.
- **Web Server:** Displays received messages and sensor data.
- **Dynamic AP Setup:** Device with the lowest MAC address sets up as an AP.
- **Real-Time Data:** View live data by connecting to the AP and navigating to `192.168.4.1`.

## Usage

1. **Upload Code:** Upload the provided code to multiple ESP32 devices.
2. **Power Up:** Power up the devices to start communication.
3. **Connect to AP:** Connect to the AP using the specified SSID and password.
4. **View Data:** Open `192.168.4.1` in a web browser to see live data.

This project showcases efficient, real-time data sharing through a mesh network of ESP32 devices with a web interface for monitoring.
