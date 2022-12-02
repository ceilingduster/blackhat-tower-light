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
#include <WiFiClientSecure.h>
#include <base64.h>
#include <DNSServer.h>
#include <Core2_Sounds.h>

// mqtt
#include <PubSubClient.h>

// project libs
#include "patternlib.h"
#include "screenlib.h"
#include "randomlibs.h"

#include <ArduinoJson.h>

// create a task handle for mqtt updates
TaskHandle_t TaskMQTTUpdate;

// wifi mac
String macaddress = WiFi.macAddress();

// mqtt root ca for hivemq
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFtTCCA52gAwIBAgIUSji7t1bcmwQat711616DcOV5j80wDQYJKoZIhvcNAQEN
BQAwajEXMBUGA1UEAwwOQW4gTVFUVCBicm9rZXIxFjAUBgNVBAoMDU93blRyYWNr
cy5vcmcxFDASBgNVBAsMC2dlbmVyYXRlLUNBMSEwHwYJKoZIhvcNAQkBFhJub2Jv
ZHlAZXhhbXBsZS5uZXQwHhcNMjIwNzI4MTUzNDI3WhcNMzIwNzI1MTUzNDI3WjBq
MRcwFQYDVQQDDA5BbiBNUVRUIGJyb2tlcjEWMBQGA1UECgwNT3duVHJhY2tzLm9y
ZzEUMBIGA1UECwwLZ2VuZXJhdGUtQ0ExITAfBgkqhkiG9w0BCQEWEm5vYm9keUBl
eGFtcGxlLm5ldDCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAL9cmwOF
Vh2ANr6jZSHQY8aBY2aSOkrjef7Y/3AQUCgQKm8QRDHb9ZgbTO3Qth2q5sc/hvDr
MEW5uSutErcRW8nUnv9SOc+Rl4fCHIQEOBD8kv2a68iSxrkkwonPS6iOkQO0lyzR
eHEC4gG8hTAkFO1wMhgH5GeyBPemIzmmXG+pYaRohS2w5vSk/fkKTtbSNA28OQ2C
tQg87J7j9O/BS3i+bZ+bangu9XZKiytyXlwUdPbvXyqNcDNmRUBL2e0NcqLouSGX
sBWJuUI6Gy0p1hmFh7nSh0rmPUQA3NegQc265UIV/vx5abz5Wha0D3b4G35sXTHd
z99oiBtLyNwhH+ChNXmn/vPKC73EsAAKeY2GuQyWLRIj+vHTSjO77nqRbKjjYaPj
Nn/EBaDUgR1L6LCXT/mwQtnKRToz5mr7Wxr9BqR+8JwMwuJgekldkDlXFC7wFKD7
7L07VNs+sIhODjn80Xt68j0NMlWfty++tGyX1d719tqN58kQmSKavUb8hqVwfaV1
oaIZcb+KSoqN+vZD7PoUc5wnpagFTbYNOAuMD10A0nMAGJaRvR5KPFYAzU5DZWY+
dGVMROTFVIz0FD6gXu8FCEy/38mavg9DCSFhzrdYUFSqncpsSmhhUgxr9cX3waXR
Kj9EI3tNxDbiaHTw2LBkHMzIkxe5AN0y/f2jAgMBAAGjUzBRMB0GA1UdDgQWBBSd
7U6kYCctCZT1nyPmXLgKMKwa3TAfBgNVHSMEGDAWgBSd7U6kYCctCZT1nyPmXLgK
MKwa3TAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBDQUAA4ICAQCRaLHI3umK
wACnQQuYLNUQbpI+cIo0nMpR+BTWL00AEhMBQov8DdvVoKLsAMFAYHMG72vROKwg
hjAT0emtpazqhjgyqSmk1jWStU7QXPTVNyyYzYaVs2fv4AypcvDj9eDUSX9EZ6y0
fO24qCyknvHEHTvJgurnS2iySSsLIxWOKKH7t2SbcraGCXtJPvGXjjj8D+d1wuyw
U+BA8PwdGYSDx7k09xaaCC9p/rtvcHhsnP6/u45Jy4aKbs8mCtTrGl0J767yy2HD
MetxR8N7uES5Gc3C7ZAVdCk8ZlMYZyOLJXSEHsiP6ogX4AnN5i2sFsJA8x7/kkzf
JLpVGGAIDFL7gcbBJP+v9FxI45BUIdmtDeqqDNARcVy5tWgQVf7tkjfjA0PU6DV2
cjtCE1f+MmmkL6E8RMxXgfUFnNGz8BaLffk8qOmtR3bTBab1Px4lgNlZhlBxDScx
FnPAIV4THfFbzw6DyrQwlfjE66SRf3wIx+FsSmZEty3pnFG7ACMCu8hrvaBWezHj
L7I6oHhpY1xr+aFmcfxxz/JicY5t5fwnunoGFoA4aXKxGjIlLm1vVMKieotArHxN
FUX4/RohJyT/8qtUu3NNp5L7cwOzsEl9xd3IR/gLVpFgH+rSTcVOL7VEkiySOkvq
jK31X4ANWZ8pKI7/0KCh56+vulJQAqcPPA==
-----END CERTIFICATE-----
)EOF";

// mqtt wifi
WiFiClientSecure espClient;
PubSubClient mqtt_client(espClient);

// callback function for the rings
void ringsComplete() {}
PatternLib rings(TOTALPIXELS, PIN, RGBTYPE + NEO_KHZ800, &ringsComplete);

// define API Key globally
String APIKey;

// mqtt server hostname
const char *mqtt_server = "mq.csta.cisco.com";

void mqtt_callback(char *topic, byte *message, unsigned int length)
{
  String payload;

  // convert byte to String
  for (int i = 0; i < length; i++)
    payload += (char)message[i];

  DynamicJsonDocument json_payload(1024);
  DeserializationError error = deserializeJson(json_payload, payload);

  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  else
  {
    // example JSON:
    // {  "to": "30:C6:F7:25:C4:38",  "p": 3,  "i": 250,  "r": 255,  "g": 0,  "b": 0,  "n": 2   }

    const String client_id = json_payload["to"];
    const int ring_number = json_payload["n"];
    const int pattern_num = json_payload["p"];
    const int interval_speed = json_payload["i"];
    const int red_value = json_payload["r"];
    const int green_value = json_payload["g"];
    const int blue_value = json_payload["b"];

    if (client_id == macaddress.c_str())
    { // is it destined for us?
      Serial.print("Received command: ");
      Serial.println(payload);

      // do the command
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
    }
  }
}

String configuration_menu()
{
  String s = String(wifi_html, wifi_html_len);
  return s;
}

String report_light_status(bool b64 = false)
{
  // for mqtt and web status msgs
  DynamicJsonDocument json_doc(1024);
  String output;

  constexpr uint32_t blue_offset{0x00};
  constexpr uint32_t green_offset{0x08}; // 8 decimal
  constexpr uint32_t red_offset{0x10};   // 16 decimal
  constexpr uint32_t byte_mask{0xFF};
  constexpr uint32_t blue_mask{byte_mask << blue_offset};
  constexpr uint32_t green_mask{byte_mask << green_offset};
  constexpr uint32_t red_mask{byte_mask << red_offset};

  for (int i = 0; i <= rings.numberRings - 1; i++)
  {
    json_doc[macaddress][i]["p"] = rings.ActivePattern[i];
    json_doc[macaddress][i]["i"] = rings.Interval[i];

    // do colours
    json_doc[macaddress][i]["c"]["r"] = (rings.pixelColor[i] & red_mask >> red_offset) & byte_mask;
    json_doc[macaddress][i]["c"]["g"] = (rings.pixelColor[i] & green_mask >> green_offset) & byte_mask;
    json_doc[macaddress][i]["c"]["b"] = (rings.pixelColor[i] & blue_mask >> blue_offset) & byte_mask;
  }
  json_doc["ti"] = (unsigned int)millis(); // cast long to int
  json_doc["sc"] = "1.0";

  serializeJson(json_doc, output);
  if (b64)
  {
    String encoded = base64::encode(output);
    return encoded;
  }
  else
  {
    return output;
  }
}

void mqtt_connect()
{
  // connect mqtt
  espClient.setCACert(root_ca);
  espClient.setInsecure();
  mqtt_client.setServer(mqtt_server, 8443);

  if (mqtt_client.connect(macaddress.c_str()))
  {
    Serial.println("Attempting to connect to MQTT broker ...");
    mqtt_client.setCallback(&mqtt_callback);
    Serial.print("Subscribing: ");
    Serial.println(mqtt_client.subscribe("towerlight/command"));
  }
  else
  {
    Serial.println("Could not connect to MQTT broker.");
  }
}

void update_mqtt()
{
  if (mqtt_client.state() != MQTT_CONNECTED)
  {
    mqtt_connect();
  }
  else
  {
    const char *output = report_light_status(true).c_str();
    mqtt_client.publish("towerlight/update", output);
    Serial.println("Updating MQTT broker ...");
  }
}

int lastUpdate = millis();
void MQTTUpdate(void *parameter)
{
  for (;;)
  {
    if (millis() - lastUpdate >= 5000)
    {
      update_mqtt();
      lastUpdate = millis();
    }
    mqtt_client.loop();
    delay(50);
  }
}

void (*resetFunc)(void) = 0;
void start_webserver()
{
  Serial.println("Starting webserver...");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", configuration_menu()); });

  server.on("/connect_wifi", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              bool res;
              if (request->hasParam("ssid"))
              {
                String ssid = request->getParam("ssid")->value();
                res = NVS.setString("ssid", ssid);
              }

              if (request->hasParam("password"))
              {
                String password = request->getParam("password")->value();
                res = NVS.setString("psk", password);
              }

              request->send(200, "text/html", "<h1>Rebooting ...</h1>");
              delay(2500);
              resetFunc(); });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
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

              request->send(200, "application/json", report_light_status()); });

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

              request->send(200, "application/json", report_light_status()); });

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

              request->send(200, "application/json", report_light_status()); });

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
              
              if (interval_speed < 100) {
                interval_speed = 100;
              } else if (interval_speed > 5000) {
                interval_speed = 5000;
              }
                

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

              // send response
              request->send(200, "application/json", report_light_status()); });

  server.onNotFound(notFound);
  server.begin();
}

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
    configurationMode = true;
    Serial.println("Starting configuration mode.");
    WiFi.softAP(defaultSSID);

    // get IP and start dns
    IPAddress IP = WiFi.softAPIP();

    // display our IP
    start_webserver();
    setMessage("http://" + IP.toString());
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }
  else
  { // otherwise connect to the wifi creds and network we have
    setMessage("Connecting ...");
    Serial.println("Retrieved wifi settings for SSID and PSK: ");
    Serial.print(nvs_ssid_config);
    Serial.print("/");
    Serial.println(nvs_psk_config);
    WiFi.begin(nvs_ssid_config.c_str(), nvs_psk_config.c_str());
    start_webserver();
  }

  mqtt_client.setKeepAlive(15);
  mqtt_client.setSocketTimeout(15);

  xTaskCreatePinnedToCore(
      MQTTUpdate,        /* Function to implement the task */
      "TaskMQTTUpdates", /* Name of the task */
      10000,             /* Stack size in words */
      NULL,              /* Task input parameter */
      0,                 /* Priority of the task */
      &TaskMQTTUpdate,   /* Task handle. */
      0);                /* Core where the task should run */

  // give it time to catch up
  delay(500);
}

void loop()
{
  lv_task_handler();
  rings.Update();
}