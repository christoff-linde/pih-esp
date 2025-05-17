# pih-esp

*Firmware for ESP32 controllers.*

## Features

- **WiFi**: Connect to WiFi networks.
- **MQTT**: Publish and subscribe to MQTT topics.

## Pre-requisites

- **ESP32**: Ensure you have an ESP32 board.
- **PlatformIO**: Install PlatformIO IDE or use the command line interface.

## Configuration

### WiFi Configuration

Edit the `src/main.cpp` file to set your WiFi credentials:

```cpp
const char* ssid = "your-SSID";
const char* password = "your-PASSWORD";
```

### MQTT Configuration

Edit the `src/main.cpp` file to set your MQTT broker details:

```cpp
const char* mqtt_server = "broker.hivemq.com";
```

### GPIO Configuration

Edit the `src/main.cpp` file to set your GPIO pins:

```cpp
# define DHTPIN D2
# define DHTTYPE DHT22
```

### Sensor Configuration

Edit the `src/main.cpp` file to set your sensor type:

```cpp
String clientID = "ESP32Client";
```

## Sensor Map

| clientID         | location    |
| ---------------- | ----------- |
| ESP32Client-0001 | Living Room |
| ESP32Client-0002 | Bedroom     |
