#include <MAX6675.h>

int CS1 = 15;              // CS pin on MAX6675
int SO1 = 16;              // SO pin of MAX6675
int SCK1 = 17;             // SCK pin of MAX6675
int units = 1;            // Units to readout temp (0 = ˚F, 1 = ˚C)
float error = 0.0;        // Temperature compensation error
float temperature = 0.0;  // Temperature output variable
MAX6675 temp0(CS1,SO1,SCK1,units,error);

float updateMax() {
     return temperature = temp0.read_temp(5);         // Read the temp 5 times and return the average value to the var
}

float getLastThermoTemp() {
   return temperature;
}
