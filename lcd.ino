#include <LiquidCrystal.h>
#include "RotaryEncoder.h"

// LiquidCrystal display with:
// rs on pin 12
// rw is pin 11
// enable on pin 6
// backlight control on pin 5 (pwm 0=bright, 511=off)
// d4, d5, d6, d7 on pins 7, 8, 9, 10
// backlight pwm control is pin 5
#define D4 7
#define D5 8
#define D6 9
#define D7 10
#define RW 11
#define E 6
#define RS 12
#define BL 5

#include "Wire.h"

LiquidCrystal lcd(RS, RW, E, D4, D5, D6, D7);
RotaryEncoder rotary(2, 19, 18);

int blevel = 80;
unsigned long startTime;

void setupLCD(float startTemp, boolean startLCD) {
  if (startLCD) {
    delay(1000);
    lcd.begin(2,16);
    lcd.clear();
    pinMode(BL, OUTPUT);
    analogWrite(BL,blevel);
  }
  startTime = millis();
  rotary.minimum(20);
  rotary.maximum(280);
  rotary.position(startTemp);
}

float updateLCD(float target, float t1, float t2) {
  float new_temp, trg;
  extern bool heaterState;
  
  new_temp = -1;
  trg = target;
  if(rotary.pressed())
  {
    Serial.println("Encoder pressed");
    new_temp = rotary.position();
    Serial.print("Encoder position = ");
    Serial.println(new_temp);
    trg = new_temp;
    startTime = millis(); // reset timer
  }
  lcd.clear();
  // first line: running time in seconds, space, temperature from IR thermometer, space, state of heater
  lcd.print((millis()-startTime)/1000.0);lcd.print(" ");lcd.print(t1);lcd.print(" ");lcd.print(getHeatCycles());
  lcd.setCursor(0,1); 
  // second line: target temperature, space, temperature from thermocouple
  lcd.print(trg); lcd.print(" ");lcd.print(t2);
  return new_temp;

}
