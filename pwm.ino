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

#include <TimerOne.h>

#define HEAT_RELAY_PIN 13

unsigned long heatcycles; // the number of millis out of 500 for the current heat amount (percent * 5)

boolean heaterState = false;
boolean highPhase = false;

unsigned long heatCurrentTime, heatOnTime, heatOffTime;

void setupHeater() {
  pinMode(HEAT_RELAY_PIN, OUTPUT);
  heaterState = false;
  heatcycles = 0;
  Timer1.initialize(500000);
  Timer1.disablePwm(HEAT_RELAY_PIN);
}


void setHeatPowerPercentage(float power) {
  unsigned long old = heatcycles;
  if (power <= 0.0) {
    power = 0.0;
  }	
  if (power >= 100.0) {
    power = 100.0;
  }
  power *= 1023; // convert to 10 bit pwm
  power /= 100.0;
  heatcycles = (unsigned long)power;
  if (heatcycles != old) {
    if (heatcycles >= 1023) {
      Timer1.disablePwm(HEAT_RELAY_PIN);
      _turnHeatElementOnOff(1); // just turn on full
    }
    else if (heatcycles) {
      Timer1.pwm(HEAT_RELAY_PIN, heatcycles, 500000);
    }
    else {
      Timer1.disablePwm(HEAT_RELAY_PIN);
      _turnHeatElementOnOff(0); // just turn off
    }
  }
}

unsigned long getHeatCycles() {
  return heatcycles;
}

void _turnHeatElementOnOff(boolean on) {
  digitalWrite(HEAT_RELAY_PIN, on);	//turn pin high
  heaterState = on;
}

// End Heater Control
