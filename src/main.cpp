#include <Arduino.h>

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

#include "DHT.h"

const char *ssid = "Main_Program_24GHZ";
const char *password = "nuncadigasnunca2007";

#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
const int DHTPin = 32; // what digital pin we're connected to

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

DHT dht(DHTPin, DHTTYPE);

float temp = 0.0, hum = 0.0;
float p = 0.0, i = 0.0, d = 0.0;
bool newPID = false;

uint32_t time_loop = 0;

String readDHT22()
{
  char auxData[512];
  memset(auxData, 0, sizeof(auxData));
  sprintf(auxData, "{\"temp\":%.2f,\"hum\":%.2f}", temp, hum);
  String tosend = String(auxData);

  return tosend;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("DHTxx test!");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", String(), false); });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", readDHT22().c_str()); });

  server.on("/data", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->params())
    {
      String p_str = request->getParam("p_value", true)->value();
      String i_str = request->getParam("i_value", true)->value();
      String d_str = request->getParam("d_value", true)->value();

      p = p_str.toFloat();
      i = i_str.toFloat();
      d = d_str.toFloat();
      newPID = true;
    }
    request->send(200,"text/plain","{\"status\":\"0k\"}"); });

  // Start server
  server.begin();

  dht.begin();
}

void loop()
{
  if (millis() - time_loop > 10000)
  {
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      temp = t;
      hum = h;
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.println(" *C ");
    }
    time_loop = millis();
  }

  if (newPID)
  {
    Serial.print("New PID parameters-> P:");
    Serial.print(p);
    Serial.print(" I: ");
    Serial.print(i);
    Serial.print(" D: ");
    Serial.println(d);
    newPID = false;
  }
}