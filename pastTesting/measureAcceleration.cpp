


#include "SparkFun_LIS331.h"
#include <Wire.h>
#include <HardwareSerial.h>

LIS331 xl;

void setup() 
{
  pinMode(9,INPUT);       // Interrupt pin input
  Wire.begin(25, 26);
  xl.setI2CAddr(0x19);
  xl.begin(LIS331::USE_I2C);

  Serial.begin(115200);
}

void loop() 
{
  static long loopTimer = 0;
  int16_t x, y, z;
  if (millis() - loopTimer > 1000)
  {
    loopTimer = millis();
    xl.readAxes(x, y, z);  // The readAxes() function transfers the
                           //  current axis readings into the three
                           //  parameter variables passed to it.
    Serial.print("X Acceleration (raw):  ");
    Serial.println(x);
    Serial.print("Y Acceleration (raw):  ");
    Serial.println(y);
    Serial.print("Z Acceleration (raw):  ");
    Serial.println(z);
    Serial.print("X Acceleration (g):  ");
    Serial.println(xl.convertToG(100,x)); // The convertToG() function
    Serial.print("Y Acceleration (g):  ");
    Serial.println(xl.convertToG(100,y)); // accepts as parameters the
    Serial.print("Z Acceleration (g):  ");
    Serial.println(xl.convertToG(100,z)); // raw value and the current
    Serial.println(" ");                // maximum g-rating.
  }
  // if (digitalRead(9) == HIGH)
  // {
  //   Serial.println("Interrupt");
  // }
}