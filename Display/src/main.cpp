#include <Arduino.h>
#include "BoardConfig.h"
#include "Version.h"

void setup()
{
    Serial.begin(115200);

    pinMode(TFT_BL,OUTPUT);
    digitalWrite(TFT_BL,HIGH);

    pinMode(LED_RED,OUTPUT);
    pinMode(LED_GREEN,OUTPUT);
    pinMode(LED_BLUE,OUTPUT);

    digitalWrite(LED_RED,HIGH);
    digitalWrite(LED_GREEN,HIGH);
    digitalWrite(LED_BLUE,HIGH);

    Serial.println();
    Serial.println(FW_NAME);
    Serial.println(FW_VERSION);
    Serial.println(FW_BUILD);

    Serial2.begin(115200,SERIAL_8N1,SENSOR_RX,SENSOR_TX);

    Serial.println("Display foundation gestart");
}

void loop()
{

}
