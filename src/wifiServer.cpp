#include <Arduino.h>

/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-relay-module-ac-web-server/

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
//#include "WiFi.h"
//#include "ESPAsyncWebServer.h"

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include "Globals.h"
#include "buttonReset.h"

// FUNCTIONS
void wifiServerLoop();
void initSPIFFS();
String readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);
bool initWiFi();
String relayState(int numRelay);
String processor(const String &var);
void setupWifiServer();

// VARIABLES
uint8_t resetToAp = 0;

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {26 /*, 24, 27, 25, 33*/};
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_SSID = "ssid";
const char *PARAM_INPUT_PASS = "pass";
const char *PARAM_INPUT_IP = "ip";
const char *PARAM_INPUT_GATEWAY = "gateway";

const char *PARAM_INPUT_1 = "relay";
const char *PARAM_INPUT_2 = "state";

// Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

IPAddress localIP;
// IPAddress localIP(192, 168, 1, 200); // hardcoded
//  Set your Gateway IP address
IPAddress localGateway;
// IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 255, 0);

// Timer variables and auxiliar
unsigned long previousMillis = 0;

const long interval = 10000; // interval to wait for Wi-Fi connection (milliseconds)
int state = 0;
// state == 0 -> connected to network
// state == 1 -> no files in SPIFFS (setting AP)
// state == 2 -> files in SPIFFS but unable to connect to network

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- frite failed");
  }
}

// Initialize WiFi
bool initWiFi()
{
  if (ssid == "" || ip == "")
  {
    state = 1;
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet))
  {
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      state = 2;
      Serial.println("Failed to connect.");
      return false;
      // PRENDER LED ROJO
    }
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    state = 0;
    Serial.println("Wifi connected");
    Serial.println(WiFi.localIP());
  }
  // Serial.println(WiFi.localIP());
  return true;
}

/********************************************************************************/
// Replaces placeholder with button section in your web page
String relayState(int numRelay)
{
  if (RELAY_NO)
  {
    if (digitalRead(relayGPIOs[numRelay - 1]))
    {
      return "";
    }
    else
    {
      return "checked";
    }
  }
  else
  {
    if (digitalRead(relayGPIOs[numRelay - 1]))
    {
      return "checked";
    }
    else
    {
      return "";
    }
  }
  return "";
}
/* *************************************************************************************************** */
String processor(const String &var)
{
  // Serial.println(var);
  if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";
    for (int i = 1; i <= NUM_RELAYS; i++)
    {
      String relayStateValue = relayState(i);
      buttons += "<h4>Relay #" + String(i) + " - DISPOSITIVO " + relayGPIOs[i - 1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" " + relayStateValue + "><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}

void setupWifiServer()
{
  initSPIFFS();
  setupButton();
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile(SPIFFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if (initWiFi())
  {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    server.serveStatic("/", SPIFFS, "/");

    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html", false, processor); });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html", false, processor); });

    //  /resetwifitoap
    server.on("/resetwifitoap", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      SPIFFS.remove("/ssid.txt");
      SPIFFS.remove("/pass.txt");
      request->send(200, "text/html", "<h1>deleted wifi credentials ssid.txt and pass.txt<br>Done.<br>ESP restart,<br>connect to AP access point ESP WIFI MANAGER <br>to configure wifi settings again<br><a href=\"http://192.168.4.1\">http://192.168.4.1</a></h1>");
      delay(5000);
      ESP.restart(); });

    //  /reboot
    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      request->send(200, "text/html", "<h1>Huh, Reboot Electra, Restart ESP32<br><a href=\"http://" + WiFi.localIP().toString()  + "\">http://" + WiFi.localIP().toString() + "</a></h1>");
      delay(5000);
      ESP.restart(); });

    /* ********************************** WIFI MANAGER 1 ************************************************************************** */
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wifimanager.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/");
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
              {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_SSID) {
            SPIFFS.remove("/ssid.txt");
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_PASS) {
            SPIFFS.remove("/pass.txt");
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_IP) {
            SPIFFS.remove("/ip.txt");
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_GATEWAY) {
            SPIFFS.remove("/gateway.txt");
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart(); });

    /* ********************************** WIFI MANAGER 2 ************************************************************************** */
    server.begin();
  }
  else
  {
    // Connect to Wi-Fi network with SSID and password
    if (state == 1)
    {
      // NULL sets an open Access Point
      WiFi.softAP("ESP-WIFI-MANAGER", NULL);
      IPAddress IP = WiFi.softAPIP();
      Serial.println("Setting AP (Access Point) : ");
      Serial.println(IP);
    }
    if (state == 2)
    {
      IPAddress IP = WiFi.localIP();
      Serial.println(IP);
    }

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/wifimanager.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/");
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
              {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_SSID) {

            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_PASS) {

            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_IP) {

            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_GATEWAY) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart(); });
    server.begin();
  }
  /**********************************************************************************************************************************/

  // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for (int i = 1; i <= NUM_RELAYS; i++)
  {
    pinMode(relayGPIOs[i - 1], OUTPUT);
    if (RELAY_NO)
    {
      digitalWrite(relayGPIOs[i - 1], HIGH);
    }
    else
    {
      digitalWrite(relayGPIOs[i - 1], LOW);
    }
  }

  /**********************************************************************************************************************************/

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if (RELAY_NO) {
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt() - 1], !inputMessage2.toInt());
      }
      else {
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt() - 1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK"); });
  // Start server
  server.begin();
}

void wifiServerLoop()
{
  buttonReset(&resetToAp);
  if (resetToAp != 0)
  {
    if (resetToAp == 1)
    {
      Serial.println("REINICIANDO DISPOSITIVO");
    }
    if (resetToAp == 2)
    {
      Serial.println("RESETEANDO DISPOSITIVO A CERO");
      SPIFFS.remove("/ssid.txt");
      SPIFFS.remove("/pass.txt");
      // request->send(200, "text/html", "<h1>deleted wifi credentials ssid.txt and pass.txt<br>Done.<br>ESP restart,<br>connect to AP access point ESP WIFI MANAGER <br>to configure wifi settings again<br><a href=\"http://192.168.4.1\">http://192.168.4.1</a></h1>");
      delay(5000);
    }
    ESP.restart();
  }
}