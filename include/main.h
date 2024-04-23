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

enum WiFiState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

#endif