#include <Arduino.h>
#include "Globals.h"

int counterTime = 0; //variable que va sumando hasta llegar a TIMERBOOT

void buttonReset(uint8_t *resetToAp);
void setupButton();

void setupButton()
{
    pinMode(BUTTONPIN, INPUT);
}
void buttonReset(uint8_t *resetToAp)
{
    if (digitalRead(BUTTONPIN))
    {
        counterTime = 0;
        delay(10);
        while (digitalRead(BUTTONPIN) && counterTime < TIMEREBOOT)
        {
            counterTime++;
            delay(1000);
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