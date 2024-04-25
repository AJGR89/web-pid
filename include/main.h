#ifndef __MAIN_H
#define __MAIN_H

/*LOGGER*/
#define ESP32DEBUGGING
#include <ESP32Logger.h>
#include "WiFi.h"
#include "secrets.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "DHT.h"
#include <PID_v1.h>

enum WiFiState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

#define PIN_INPUT 36
#define RELAY_PIN 2

#endif