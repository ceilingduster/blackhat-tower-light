#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

// functions for the webserver
AsyncWebServer server(80);
String generateAPIKey();
String IpAddress2String;
void notFound(AsyncWebServerRequest *request);
void serverError(AsyncWebServerRequest *request);