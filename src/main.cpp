#include <Arduino.h>
#include "main.h"

#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
const int DHTPin = 32; // what digital pin we're connected to

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");

DHT dht(DHTPin, DHTTYPE);

float temp = 0.0, hum = 0.0;
float p = 0.0, i = 0.0, d = 0.0;
bool newPID = false;

uint32_t time_loop = 0;

// WiFi connections FSM
WiFiState wifiState = DISCONNECTED;
unsigned long lastAttemptTime = 0;
const unsigned long attemptInterval = 1000;
int connect_count = 0;

String readDHT22(void);
void initSPIFFS(void);
void initWiFi(void);
void handleWiFiConnection(void);

void setup()
{
  Serial.begin(115200);
  DBGINI(&Serial, ESP32Timestamp::TimestampNone);
  DBGSTA
  DBGLEV(Debug);

  DBGLOG(Info, "PID -Web config start!!");

  initWiFi();
  initSPIFFS();

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
  handleWiFiConnection();

  if (millis() - time_loop > 10000)
  {
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
      DBGLOG(Error, "Failed to read from DHT sensor!");
    }
    else
    {
      temp = t;
      hum = h;
      DBGLOG(Debug, "Humidity: %.2f %\tTemperature: %.2f ºC", hum, temp);
    }
    time_loop = millis();
  }

  if (newPID)
  {
    DBGLOG(Debug, "New PID parameters-> P: %.2f, I: %.2f, D: %.2f", p, i, d);
    newPID = false;
  }
}

String readDHT22()
{
  char auxData[512];
  memset(auxData, 0, sizeof(auxData));
  sprintf(auxData, "{\"temp\":%.2f,\"hum\":%.2f}", temp, hum);
  String tosend = String(auxData);

  return tosend;
}

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    DBGLOG(Error, "An error has occurred while mounting SPIFFS");
  }
  DBGLOG(Info, "SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);

  DBGLOG(Info, "Connecting to WiFi (...)");
  wifiState = CONNECTING;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }
  wifiState = CONNECTED;
  IPAddress ip = WiFi.localIP();
  String s = ip.toString();
  DBGLOG(Debug, "WiFi connected, IP address: %s", s.c_str());
}

void handleWiFiConnection()
{
  static uint8_t first_connect = 0;
  static unsigned long lastDisconnectTime = 0;

  switch (wifiState)
  {
  case DISCONNECTED:
    DBGLOG(Debug, "Connecting to:  %s with: %s", SSID, PASS);
    WiFi.begin(SSID, PASS);
    wifiState = CONNECTING;
    lastAttemptTime = millis();
    break;

  case CONNECTING:
    if (WiFi.status() == WL_CONNECTED)
    {
      wifiState = CONNECTED;
      DBGLOG(Info, "   WiFi connected");
      IPAddress ip = WiFi.localIP();
      String s = ip.toString();
      DBGLOG(Debug, "WiFi connected, IP address: %s", s.c_str());
    }
    else if (millis() - lastAttemptTime >= attemptInterval)
    {
      lastAttemptTime = millis();
      connect_count++;
      if (connect_count > 180)
      {
        wifiState = DISCONNECTED;
        connect_count = 0;
      }
    }
    break;

  case CONNECTED:
    // If you need to do something periodically when you're online, you can do it here.
    if (WiFi.status() != WL_CONNECTED && millis() - lastDisconnectTime > 30000)
    {
      DBGLOG(Info, "WiFi disconnected for more than 30 seconds. Reconnecting...");
      wifiState = DISCONNECTED;
      lastDisconnectTime = millis(); // Update disconnection time
    }

    if (!first_connect)
    {
      first_connect = 1;
    }
    break;
  }
}