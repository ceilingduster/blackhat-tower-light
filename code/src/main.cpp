#include <M5Core2.h>
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoNvs.h>
#include <PubSubClient.h>

// project libs
#include "patternlib.h"
#include "screenlib.h"
#include "randomlibs.h"

#include <ArduinoJson.h>

DynamicJsonDocument json_doc(1024);

// callback function for the rings
void ringsComplete() {}
PatternLib rings(TOTALPIXELS, PIN, RGBTYPE + NEO_KHZ800, &ringsComplete);

// setup a wificlient for the pubsub client
WiFiClient wifi_pubSub;
PubSubClient pubsub_Client(wifi_pubSub);

// define API Key globally
String APIKey;

String report_light_status()
{
  String output;
  json_doc["ring1_pattern"] = rings.ActivePattern[0];
  json_doc["ring2_pattern"] = rings.ActivePattern[1];
  json_doc["ring3_pattern"] = rings.ActivePattern[2];

  json_doc["ring1_interval"] = rings.Interval[0];
  json_doc["ring2_interval"] = rings.Interval[1];
  json_doc["ring3_interval"] = rings.Interval[2];

  json_doc["ring1_color"]["green"] = rings.Green(rings.pixelColor[0]);
  json_doc["ring1_color"]["red"] = rings.Red(rings.pixelColor[0]);
  json_doc["ring1_color"]["blue"] = rings.Blue(rings.pixelColor[0]);

  json_doc["ring2_color"]["green"] = rings.Green(rings.pixelColor[1]);
  json_doc["ring2_color"]["red"] = rings.Red(rings.pixelColor[1]);
  json_doc["ring2_color"]["blue"] = rings.Blue(rings.pixelColor[1]);

  json_doc["ring3_color"]["green"] = rings.Green(rings.pixelColor[2]);
  json_doc["ring3_color"]["red"] = rings.Red(rings.pixelColor[2]);
  json_doc["ring3_color"]["blue"] = rings.Blue(rings.pixelColor[2]);

  serializeJson(json_doc, output);
  return output;
}

void start_webserver()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "application/json", report_light_status()); });

 
  server.on("/light_on", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // get ring number
              long ring_number;
              if (request->hasParam("n"))
              {
                ring_number = request->getParam("n")->value().toInt();
              }
              else
              {
                ring_number = 0;
              }

              rings.pixelColor[ring_number] = rings.Color(255, 255, 255);
              rings.Interval[ring_number] = 100;
              rings.ActivePattern[ring_number] = rings.GetPattern(3); // FILL

              if (rings.ActivePattern[ring_number] != 0)
              {
                updateRing(ring_number, String(rings.ActivePattern[ring_number]).c_str());
              }
              else
              {
                updateRing(ring_number, "");
              }

              request->send(200, "application/json", report_light_status());
            });

  server.on("/light_off", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // get ring number
              long ring_number;
              if (request->hasParam("n"))
              {
                ring_number = request->getParam("n")->value().toInt();
              }
              else
              {
                ring_number = 0;
              }

              rings.pixelColor[ring_number] = rings.Color(0, 0, 0);
              rings.Interval[ring_number] = 100;
              rings.ActivePattern[ring_number] = rings.GetPattern(0); // FILL

              if (rings.ActivePattern[ring_number] != 0)
              {
                updateRing(ring_number, String(rings.ActivePattern[ring_number]).c_str());
              }
              else
              {
                updateRing(ring_number, "");
              }

              request->send(200, "application/json", report_light_status());
            });

  server.on("/light_toggle", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // get ring number
              long ring_number;
              if (request->hasParam("n"))
              {
                ring_number = request->getParam("n")->value().toInt();
              }
              else
              {
                ring_number = 0;
              }

              rings.Interval[ring_number] = 100;

              if (rings.ActivePattern[ring_number] == 0)
              {
                rings.ActivePattern[ring_number] = rings.GetPattern(3); // FILL/ON
                rings.pixelColor[ring_number] = rings.Color(255, 255, 255);
              }
              else
              {
                rings.ActivePattern[ring_number] = rings.GetPattern(0); // OFF
                rings.pixelColor[ring_number] = rings.Color(0, 0, 0);
              }

              if (rings.ActivePattern[ring_number] != 0)
              {
                updateRing(ring_number, String(rings.ActivePattern[ring_number]).c_str());
              }
              else
              {
                updateRing(ring_number, "");
              }

              request->send(200, "application/json", report_light_status());
            });

  server.on("/control", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // get ring number
              long ring_number;
              if (request->hasParam("n"))
              {
                ring_number = request->getParam("n")->value().toInt();
              }
              else
              {
                ring_number = 0;
              }
              if (ring_number > NUMRINGS || ring_number < 0)
                ring_number = 0;

              // get red value
              long red_value;
              if (request->hasParam("r"))
              {
                red_value = request->getParam("r")->value().toInt();
              }
              else
              {
                red_value = 0;
              }
              if (red_value > 255 || red_value < 0)
                red_value = 0;

              // get blue value
              long blue_value;
              if (request->hasParam("b"))
              {
                blue_value = request->getParam("b")->value().toInt();
              }
              else
              {
                blue_value = 0;
              }
              if (blue_value > 255 || blue_value < 0)
                blue_value = 0;

              // get green value
              long green_value;
              if (request->hasParam("g"))
              {
                green_value = request->getParam("g")->value().toInt();
              }
              else
              {
                green_value = 0;
              }
              if (green_value > 255 || green_value < 0)
                green_value = 0;

              // get pattern number
              long pattern_num;
              if (request->hasParam("p"))
              {
                pattern_num = request->getParam("p")->value().toInt();
              }
              else
              {
                pattern_num = 0;
              }
              if (pattern_num > 7 || pattern_num < 0)
                pattern_num = 0;

              // get interval speed
              long interval_speed;
              if (request->hasParam("i"))
              {
                interval_speed = request->getParam("i")->value().toInt();
              }
              else
              {
                interval_speed = 500;
              }
              if (interval_speed > 5000 || interval_speed < 100)
                pattern_num = 500;

              rings.pixelColor[ring_number] = rings.Color(red_value, green_value, blue_value);
              rings.Interval[ring_number] = interval_speed;
              rings.ActivePattern[ring_number] = rings.GetPattern(pattern_num);

              if (rings.ActivePattern[ring_number] != 0)
              {
                updateRing(ring_number, String(rings.ActivePattern[ring_number]).c_str());
              }
              else
              {
                updateRing(ring_number, "");
              }

              request->send(200, "application/json", report_light_status());
            });

  server.onNotFound(notFound);
  server.begin();
}

// for storing the nvs values
String nvs_ssid_config;
String nvs_psk_config;

// on connection, write nvs values
static void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Saving WiFi NVS SSID and PSK.");

    // Save values to NVS
    bool res;
    res = NVS.setString("ssid", WiFi.SSID());
    res = NVS.setString("psk", WiFi.psk());

    setMessage(""); // wifi is connected, clear the spinner label
    start_webserver();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    setMessage("Reconnecting ...");
    WiFi.reconnect();
    break;
  }
}

void setup()
{
  // initialize NVS
  NVS.begin();

  // initialize the screen
  tft_lv_initialization();
  init_disp_driver();
  init_touch_driver();
  start_screen_task();

  // generate an API Key
  APIKey = generateAPIKey();

  WiFi.mode(WIFI_AP_STA);
  WiFi.onEvent(WiFiEvent);
  WiFi.setAutoReconnect(true); // auto reconnect

  nvs_ssid_config = NVS.getString("ssid");
  nvs_psk_config = NVS.getString("psk");

  // if no nvs settings, start smart config
  if (nvs_ssid_config == NULL || nvs_ssid_config == "")
  {
    Serial.println("Starting Smart Config.");
    setMessage("Use Smart Config on your phone.");
    WiFi.beginSmartConfig();
  }
  else
  { // otherwise connect to the wifi creds and network we have
    setMessage("Connecting ...");
    Serial.println("Retrieved wifi settings for SSID and PSK: ");
    Serial.print(nvs_ssid_config);
    Serial.print("/");
    Serial.println(nvs_psk_config);
    WiFi.begin(nvs_ssid_config.c_str(), nvs_psk_config.c_str());
  }
}

void loop()
{
  lv_task_handler();
  rings.Update();

  // report the light status (convert char to String)
  String json_doc_status = report_light_status();
  char json_doc_char[1024];
  json_doc_status.toCharArray(json_doc_char, 1024);
  pubsub_Client.publish("json_status", json_doc_char);
}