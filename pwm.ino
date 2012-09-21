// HeaterControl
// Tim Hirzel 
// Dec 2007
// 
// This file is for controlling a heater via a solid state zero crossing relay

// since these are zero-crossing relays, it makes sense to just match my local
// AC frequency, 60hz
//
// All code released under
// Creative Commons Attribution-Noncommercial-Share Alike 3.0 

#define HEAT_RELAY_PIN 13

float heatcycles; // the number of millis out of 1000 for the current heat amount (percent * 10)

boolean heaterState = 0;

unsigned long heatCurrentTime, heatOnTime, heatOffTime;

void setupHeater() {
  pinMode(HEAT_RELAY_PIN , OUTPUT);
  heatOnTime = 0;
  heatOffTime = 0;
}


void updateHeater() {
  heatCurrentTime = millis();
  if (heatCurrentTime >= heatOnTime) {
    heatOnTime = heatCurrentTime + 500;
    heatOffTime = heatCurrentTime + (unsigned long)heatcycles;
    // begin cycle
    if (heatcycles > 0.0) {
      _turnHeatElementOnOff(1);
//      Serial.print("on=");
//      Serial.print(heatOnTime);
//      Serial.print(" off=");
//      Serial.println(heatOffTime);
    }
  } 
  if (heatCurrentTime >= heatOffTime) {
    _turnHeatElementOnOff(0);
  }
}

void setHeatPowerPercentage(float power) {
  if (power <= 0.0) {
    power = 0.0;
  }	
  if (power >= 500.0) {
    power = 500.0;
  }
  heatcycles = power;
}

float getHeatCycles() {
  return heatcycles;
}

void _turnHeatElementOnOff(boolean on) {
  digitalWrite(HEAT_RELAY_PIN, on);	//turn pin high
  //if (heaterState != on)
  //  Serial.println(on ? "HEATER ON" : "HEATER OFF");
  heaterState = on;
}

// End Heater Control
