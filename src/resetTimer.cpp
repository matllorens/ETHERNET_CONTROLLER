#include <Arduino.h>
#include "esp_system.h"

#define MINUTES 5 

hw_timer_t *timer = NULL;
volatile int interruptCounter;
int totalInterruptCounter;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer();
void WTDloop();
void setupWTD();

void setupWTD()
{
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000, true); //Every 1 second
    timerAlarmEnable(timer);
}
void WTDloop()
{
    if (interruptCounter > 0)
    {
        portENTER_CRITICAL(&timerMux);
        interruptCounter--;
        portEXIT_CRITICAL(&timerMux);

        totalInterruptCounter++;

        Serial.print("An interrupt as occurred. Total number: ");
        Serial.println(totalInterruptCounter);
        if (totalInterruptCounter >= MINUTES*60)
        {
            ESP.restart();
        }
    }
}

void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    interruptCounter++;
    portEXIT_CRITICAL_ISR(&timerMux);
}