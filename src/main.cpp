#include "secrets.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <DHT_U.h>
#include "Arduino.h"
#include <time.h>

#define DHTPIN D2
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *server_url = SERVER_URL;

const char *mqtt_server = MQTT_SERVER;

WiFiClient espClient;
HTTPClient httpClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Create a new ID for each ESP client device
String clientId = CLIENT_ID;

unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 3600000; // 1 hour in ms

void initWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void networkScan()
{
  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found
  int numberOfNetworks = WiFi.scanNetworks();

  if (numberOfNetworks == 0)
  {
    Serial.println("No networks found");
  }
  else
  {
    Serial.print(numberOfNetworks);
    Serial.println(" networks found");
    for (int index = 0; index < numberOfNetworks; ++index)
    {
      // Print SSID and RSSI for each network found
      Serial.print(index + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(index));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(index));
      Serial.print(")");
      Serial.println(WiFi.encryptionType(index) == WIFI_AUTH_OPEN ? " " : "*");
      delay(10);
    }
  }
  Serial.println("Scan done");
  Serial.println("");
}

void initSensors()
{
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("°C"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("°C"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print(F("Sensor Type: "));
  Serial.println(sensor.name);
  Serial.print(F("Driver Ver:  "));
  Serial.println(sensor.version);
  Serial.print(F("Unique ID:   "));
  Serial.println(sensor.sensor_id);
  Serial.print(F("Max Value:   "));
  Serial.print(sensor.max_value);
  Serial.println(F("%"));
  Serial.print(F("Min Value:   "));
  Serial.print(sensor.min_value);
  Serial.println(F("%"));
  Serial.print(F("Resolution:  "));
  Serial.print(sensor.resolution);
  Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void initMqtt()
{
  Serial.println("Init MQTT");
  // Set MQTT properties
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect()
{
  Serial.println("Reconnecting...");
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);

  Serial.println("Starting setup for ESP32 Client:" + clientId);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  networkScan();
  initWifi();
  initSensors();
  // TODO: re-enable MQTT if needed
  // initMqtt();

  configTime(7200, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for NTP time sync...");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  Serial.println("Time synchronized");

  Serial.println("Setup done");
}

void loop()
{
  unsigned long now = millis();

  if (now - lastNtpSync > ntpSyncInterval) {
    configTime(7200, 0, "pool.ntp.org", "time.nist.gov");
    lastNtpSync = now;
    Serial.println("NTP time re-synced");
  }

  if (now - lastMsg > delayMS)
  {
    lastMsg = now;

    StaticJsonDocument<200> doc;
    // add all data into array in the following format: [{ ...JSON_DATA... }]
    doc[0]["device_id"] = clientId;

    // store UTC timestamp as integer (seconds since epoch)
    time_t rawtime;
    struct tm timeinfo;
    time(&rawtime);
    doc[0]["timestamp"] = (long)rawtime;

    sensors_event_t event;
    dht.temperature().getEvent(&event);
    float temperature = event.temperature;
    if (isnan(temperature))
    {
      Serial.println(F("Error reading temperature!"));
    }
    else
    {
      Serial.print(F("Temperature: \t"));
      Serial.print(temperature);
      Serial.println(F("°C"));
      doc[0]["temperature"] = temperature;
    }

    dht.humidity().getEvent(&event);
    float humidity = event.relative_humidity;
    if (isnan(humidity))
    {
      Serial.println(F("Error reading humidity!"));
    }
    else
    {
      Serial.print(F("Humidity: \t"));
      Serial.print(humidity);
      Serial.println(F("%"));
      doc[0]["humidity"] = humidity;
    }

    char json[200];
    serializeJson(doc, json);
    Serial.print("JSON Output: ");
    Serial.println(json);
    // TODO: re-enable MQTT publish if needed
    // client.publish("pih", json);
    httpClient.begin(server_url);
    httpClient.addHeader("Content-Type", "application/json");
    int httpResponseCode = httpClient.POST(json);
    if (httpResponseCode > 0)
    {
      String response = httpClient.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    httpClient.end();
  }
}
