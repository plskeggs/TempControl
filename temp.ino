

#define IR_CLK 3
#define IR_DATA 4

volatile int nbits = 0;
volatile  byte hexbyte = 0;
volatile  byte read_byte;
volatile int byte_ready = 0;

volatile unsigned char message[4];
volatile int nbytes = 0;
volatile int message_waiting = 0;

unsigned long last_time = 0;

float temp= 99.0;
float ambient;


float tcSum = 0.0;
float latestReading = 0.0;
int readCount = 0;
float multiplier;
void setupTempSensor() {
  pinMode(IR_CLK, INPUT);
  pinMode(IR_DATA, INPUT);
  attachInterrupt(1, readBit, FALLING);
}  

void updateTempSensor() {
    if (message_waiting == 1) {
      last_time = millis();
      if (message[0] == 0x4c) {
      int t = message[1]<<8 | message[2];
      temp = t/16.0 -273.15;
    } else if (message[0] == 0x66) {
      int t = message[1]<<8 | message[2];
      ambient = t/16.0 -273.15;
    }
    message_waiting = 0;
  }
  tcSum += temp;
  readCount +=1;

  if (millis() - last_time > 1000) {
    nbits = 0;
    nbytes = 0;
    hexbyte = 0;
    message_waiting = 0;
    byte_ready = 0;
    last_time = millis();
  }
}
// Interupt routine for handling IR sensor clock trailing edge
void readBit() {
  int val = digitalRead(IR_DATA);
  nbits++;
  int bit = (val == HIGH) ? 1 : 0;
  hexbyte = (hexbyte << 1) | bit;
  if (nbits == 8) {
    if (byte_ready == 0) {
      read_byte = hexbyte;
      byte_ready = 1;
    }
    if (hexbyte == 0xd) {
      nbytes = 0;
      message_waiting = 1;
    } else if (message_waiting == 0) {
      if (nbytes < 4) {
        message[nbytes] = hexbyte;
      }
      nbytes++;
    }
    hexbyte = 0;
    nbits = 0;
  }
}

float getFreshTemp() { 
      latestReading = temp;
      readCount = 0;
      tcSum = 0.0;
  return latestReading;

}

float getLastTemp() {
  return latestReading;

}

// END Temperature Sensor
