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

typedef enum menu_state
{
   MS_INIT, MS_IDLE, MS_MANUAL, MS_RUNNING, MS_DONE
} MENU_STATE;

MENU_STATE menuState = MS_INIT;

typedef struct tempProfilePoint
{
   float goalTemp;
   unsigned long timeToReach; // in seconds
} TEMP_PROFILE_POINT;

TEMP_PROFILE_POINT leadProfile[] = {
   {100.0, 45}, // pre-heat
   {140.0, 45}, // flux activation
   {180.0, 120},// soaking to minimize voiding in BGA assemblies
   {220.0, 30}, // reflow!
   {180.0, 30}, // cooling starts now... open the door to help it along, since this thing is a heater, not a cooler
   {25.0, 120},
   {0.0, 0}     // end of profile sentinel
};

TEMP_PROFILE_POINT leadFreeProfile[] = {
   {100.0, 60}, // pre-heat
   {140.0, 60}, // flux activation
   {219.0, 90}, // soak
   {240.0, 30}, // peak reflow (min 230, max 249)
   {219.0, 30}, // cooling start
   {25.0, 100},
   {0.0, 0}     // end of profile sentinel
};

TEMP_PROFILE_POINT *curProfile;
float currentSlope; // degrees per millisecond

int blevel = 80;
unsigned long startTime;
unsigned long endTime;
unsigned long lcdUpdateTime;
unsigned long tempUpdateTime;

const int menuX[4] = {
   1, 5, 9, 13
};
int mode = 0;

void setupLCD(float startTemp, boolean startLCD) {
  if (startLCD) {
    delay(1000);
    lcd.begin(2,16);
    lcd.clear();
    pinMode(BL, OUTPUT);
    analogWrite(BL,blevel);
    lcd.print("TempControl v1.0");
    delay(2000);
  }
  startTime = millis();
  rotary.minimum(20);
  rotary.maximum(280);
  rotary.position(startTemp);
  menuState = MS_INIT;
  curProfile = NULL;
}

float updateLCD(float target, float t1, float t2) {
  float new_temp, trg;
  extern bool heaterState;
  unsigned long curTime;
  int old_mode;
  
  new_temp = -1;
  trg = target;

  switch(menuState)
  {
     case MS_INIT:
        lcd.clear();
        lcd.print("Off Man  Pb NoPb");
        lcd.setCursor(menuX[mode],1);
        lcd.print("^");
        lcd.blink();
        menuState = MS_IDLE;
        break;
     case MS_IDLE:
        if(!rotary.pressed())
        {
           old_mode = mode;
           mode = rotary.position() % 4;
           if(mode != old_mode)
           {
              lcd.setCursor(menuX[old_mode],1);
              lcd.print(" ");
              lcd.setCursor(menuX[mode],1);
              lcd.print("^");
           }
        }
        else // pressing the encoder selects the menu item
        {
           lcd.noBlink();
           if(mode == 0)
           {
              trg = new_temp = 0; // setting temperature to 0 effectively turns the unit off
              startTime = millis();
              menuState = MS_MANUAL;
              Serial.println("Off Mode");
              lcd.clear();
              lcd.print("Off Mode");
              delay(2000);
           }
           else if(mode == 1)
           {
              trg = new_temp = readFloat(ESPRESSO_TEMP_ADDRESS); // from EEPROM. load the saved value
              startTime = millis();
              menuState = MS_MANUAL;
              Serial.println("Manual Mode");
              lcd.clear();
              lcd.print("Manual Mode");
              delay(2000);
           }
           else if(mode == 2) // lead profile
           {
              curProfile = &leadProfile[0];
              startTime = tempUpdateTime = millis();
              endTime = startTime + curProfile->timeToReach * 1000;
              trg = new_temp = getCurrentTemp();
              menuState = MS_RUNNING;
              Serial.println("Lead Profile");
              lcd.clear();
              lcd.print("Lead Profile");
              delay(2000);
           }
           else if(mode == 3) // lead-free profile
           {
              curProfile = &leadFreeProfile[0];
              startTime = tempUpdateTime = millis();
              endTime = startTime + curProfile->timeToReach * 1000;
              trg = new_temp = getCurrentTemp();
              menuState = MS_RUNNING;
              Serial.println("Lead-Free Profile");
              lcd.clear();
              lcd.print("LeadFree Profile");
              delay(2000);
           }
        }
        break;
     case MS_MANUAL:
        curTime = millis();
        if(rotary.pressed())
        {
          Serial.println("Encoder pressed");
          new_temp = rotary.position();
          Serial.print("Encoder position = ");
          Serial.println(new_temp);
          trg = new_temp;
          startTime = curTime; // reset timer
          lcdUpdateTime = curTime - 1; // force update to LCD below, so update is responsive when user is changing temperature
        }
        if(curTime > lcdUpdateTime)
        {
           lcd.clear();
           // first line: running time in seconds, space, temperature from IR thermometer, space, state of heater
           lcd.print((millis()-startTime)/1000.0);lcd.print(" ");lcd.print(t1);lcd.print(" ");lcd.print(getHeatCycles());
           lcd.setCursor(0,1); 
           // second line: target temperature, space, temperature from thermocouple
           lcd.print(trg); lcd.print(" ");lcd.print(t2);
           lcdUpdateTime = curTime + 1000;
        }
        break;
     case MS_RUNNING:
        curTime = millis();
        if(curTime < endTime)
        {
           currentSlope = (curProfile->goalTemp - getCurrentTemp()) / (endTime - curTime);
           trg = new_temp = target + currentSlope * (curTime - tempUpdateTime);
           tempUpdateTime = curTime;
           if(curTime > lcdUpdateTime)
           {
              lcd.clear();
              // first line: running time in seconds, space, end time, space, state of heater
              lcd.print((curTime-startTime)/1000.0);lcd.print(" ");lcd.print((endTime-startTime)/1000.0);lcd.print(" ");lcd.print(getHeatCycles());
              lcd.setCursor(0,1); 
              // second line: target temperature, space, temperature from active temperature sensor, space, goal temp
              lcd.print(trg); lcd.print(" ");lcd.print(getCurrentTemp());lcd.print(" "); lcd.print(curProfile->goalTemp);
              lcdUpdateTime = curTime + 1000;
           }
        }
        else
        {
           Serial.println("next profile");
           curProfile++;
           startTime = curTime;
           endTime = curTime + curProfile->timeToReach * 1000;
           Serial.print("time to reach=");
           Serial.print(curProfile->timeToReach);
           Serial.print(", goal temp=");
           Serial.println(curProfile->goalTemp);
           if(curProfile->timeToReach == 0)  // reached the end of the profile table
           {
              Serial.println("done with profile");
              lcd.clear();
              lcd.print("Done.  Cur temp:");
              lcd.setCursor(0,1);
              lcd.print(getCurrentTemp());
              menuState = MS_DONE;
              new_temp = 0;
              curProfile = NULL;
           }
        }
        break;
     case MS_DONE:
        if(rotary.pressed()) // user can also press main button to reset back to menu
           menuState = MS_INIT;
        break;
  }
  return new_temp;

}
