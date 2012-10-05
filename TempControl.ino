// BBCC Main
// Tim Hirzel
// February 2008
//
// Main file for the Bare Bones Coffee Controller PID 
// setup for Arduino.
// This project is set up such that each tab acts as a 
// "module" or "library" that incporporates some more 
// functionality.  Each tab correlates 
// to a particular device (Nunchuck),  protocol (ie. SPI), 
// or Algorithm (ie. PID).

// The general rule for any of these tabs/sections is that 
// if they include a setup* or update* function, those should be added
// into the main setup and main loop functions. Also, in main loop, and in 
// extra code, delays should probably be avoided.
// Instead, use millis() and check for a certain interval to have passed.
//
// All code released under
// Creative Commons Attribution-Noncommercial-Share Alike 3.0 
// Modifications for hot plate SMD reflow by Scott Dixon, March 2010.


// These are addresses into EEPROM memory.  The values to be stores are floats which 
// need 4 bytes each.  Thus 0,4,8,12,...
#define PGAIN_ADR 0
#define IGAIN_ADR 4
#define DGAIN_ADR 8
#define RESET_BUTTON 14

#define ESPRESSO_TEMP_ADDRESS 12
//#define STEAM_TEMP_ADDRESS 12  // steam temp currently not used with bare bones setup

#define PID_UPDATE_INTERVAL 250 // milliseconds

// comment out the following to use the IR thermometer for control
#define USE_THERMOCOUPLE 1

float targetTemp;  //current temperature goal
float heatPower; // 0 - 500  milliseconds on per second
float thermo_temp;

unsigned long lastPIDTime;  // most recent PID update time in ms 

void setup()
{
  setupSerialInterface();
  Serial.println(F("Temperature Controller v1.1"));
  Serial.println(F("Setting up reset button..."));
  pinMode(RESET_BUTTON, INPUT);
  digitalWrite(RESET_BUTTON, HIGH);
  Serial.println(F("Setting up PID..."));
  setupPID(PGAIN_ADR, IGAIN_ADR, DGAIN_ADR ); // Send addresses to the PID module 
  Serial.println(F("Reading EEPROM..."));
  targetTemp = readFloat(ESPRESSO_TEMP_ADDRESS); // from EEPROM. load the saved value
  Serial.println(F("Setting up LCD..."));
  setupLCD(targetTemp, true);
  lastPIDTime = millis();
  // module setup calls
  Serial.println(F("Setting up heater..."));
  setupHeater();
  Serial.println(F("Setting up temperature sensor..."));
  setupTempSensor();
  Serial.println(F("Setup complete.  Starting."));
  printHelp();
}

void setTargetTemp(float t) {
  targetTemp = t;
  writeFloat(t, ESPRESSO_TEMP_ADDRESS);
}

float getTargetTemp() {
  return targetTemp;
}

float getCurrentTemp() {     // get which ever temperature is the controlling one
#if defined(USE_THERMOCOUPLE)
   return getLastThermoTemp();
#else
   return getLastTemp();
#endif
}

void loop()
{  
  float new_temp;
  float ir_temp;
  
  if (digitalRead(RESET_BUTTON) == LOW) {
      Serial.println(F("button pressed"));
      targetTemp = readFloat(ESPRESSO_TEMP_ADDRESS); // from EEPROM. load the saved value
      setupLCD(targetTemp, false);
      Serial.print(F("setting target temp = "));
      Serial.println(targetTemp);
  }
  // this call interprets characters from the serial port
  // its a very basic control to allow adjustment of gain values, and set temp
  updateSerialInterface(); 
#if !defined(USE_THERMOCOUPLE)
  updateTempSensor();
#endif

  // every second, udpate the current heat control, and print out current status

  // This checks for rollover with millis()
  if (millis() < lastPIDTime) {
    lastPIDTime = 0;
  }

  if ((millis() - lastPIDTime) > PID_UPDATE_INTERVAL) {
#if defined(USE_THERMOCOUPLE)
    ir_temp = 0.0;
#else
    ir_temp = getLastTemp();
#endif
    new_temp = updateLCD(targetTemp, ir_temp, thermo_temp);
    //new_temp = 100;
    if (new_temp >= 0) {
      targetTemp = new_temp;
    }
    lastPIDTime +=  PID_UPDATE_INTERVAL;
 #if defined(USE_THERMOCOUPLE)
    heatPower = updatePID(targetTemp, thermo_temp);
 #else
    heatPower = updatePID(targetTemp, getFreshTemp());
 #endif
    setHeatPowerPercentage(heatPower);
    thermo_temp = updateMax();
  }  
}

// END BBCC Main
