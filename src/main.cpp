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
#include "Globals.h"
#include "buttonReset.h"
#include "wifiServer.h"

/************** VARIABLES AND DECLARATIONS CORE 2 ***************************/

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  setupWifiServer();
}
void loop()
{
  wifiServerLoop();
}