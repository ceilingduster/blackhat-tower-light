#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/html", "<html><head><title>Not Found</title></head><body><h1>Not Found</h1></body></html>");
}

void serverError(AsyncWebServerRequest *request)
{
  request->send(500, "text/html", "<html><head><title>Server Error</title></head><body><h1>Server Error</h1></body></html>");
}

String generateAPIKey()
{
  String APIKey;
  for (int i = 0; i < 10; i++)
  {
    int charFlip = random(0, 3);
    char myCharacter;

    switch (charFlip)
    {
    case 0:
      myCharacter = random(97, 122);
      break; // a-z
    case 1:
      myCharacter = random(65, 90);
      break; // A-Z
    case 2:
      myCharacter = random(48, 57);
      break; // 0-9
    }
    APIKey += String(myCharacter);
  }

  return APIKey;
}

String IpAddress2String(const IPAddress &ipAddress)
{
  return String(ipAddress[0]) + String(".") +
         String(ipAddress[1]) + String(".") +
         String(ipAddress[2]) + String(".") +
         String(ipAddress[3]);
}
