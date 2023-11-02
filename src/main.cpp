#include "WiFi.h"
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D2
#define DHTTYPE DHT22

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

const char *ssid = "";
const char *password = "";

HTTPClient http;
String serverName = "http://192.168.0.101:8000/api/client/v1/update-sensor";

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

void setup()
{
  delay(100);
  Serial.begin(115200);
  Serial.println("Starting setup");

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  networkScan();
  initWifi();
  initSensors();
  Serial.println("Setup done");
}

void loop()
{
  delay(delayMS);

  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temperature = event.temperature;
  if (isnan(temperature))
  {
    Serial.println(F("Error reading temperature!"));
  }
  else
  {
    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C"));
  }

  dht.humidity().getEvent(&event);
  float humidity = event.relative_humidity;
  if (isnan(humidity))
  {
    Serial.println(F("Error reading humidity!"));
  }
  else
  {
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("%"));
  }

  String serverPath = serverName + "?sensor_id=DHT22_01&temperature=" + String(temperature) + "&humidity=" + String(humidity);
  http.begin(serverPath.c_str());

  int httpResponseCode = http.GET();
}
