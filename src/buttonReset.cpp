#include <Arduino.h>
#include "Globals.h"
const int buttonPin = 27;
int counterTime = 0;

void buttonReset(uint8_t *resetToAp);

void setupButton()
{
    pinMode(buttonPin, INPUT);
}
void buttonReset(uint8_t *resetToAp)
{
    if (digitalRead(buttonPin))
    {
        counterTime = 0;
        delay(10);
        while (digitalRead(buttonPin) && counterTime < TIMEREBOOT)
        {
            counterTime++;
            Serial.println(counterTime);
        }
        if (counterTime >= TIMEREBOOT)
        {
            *resetToAp = 2; // RESET DE DISPOSITIVO
        }
        else
        {
            *resetToAp = 1; // REINICIO DE DISPOSITIVO
        }
    }
}