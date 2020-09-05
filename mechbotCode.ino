//Included Libraries
#include <mechbotShield.h>
#include <avr/io.h>

//Constants
#define FASTMVMT 550
#define SLOWMVMT 350
#define NOMVMT 0

//Global Variables
char distMsg;           //If using char msg, S=STOP, F=FAR

void setup() {
  Serial.begin(19200);				//Higher baud rate recommended for faster response
  disableMotor();
  initMotor();
  initADC();
}

void loop() {
  if (Serial.available() > 0) {		//Reads from serial port IF there is something to read
    distMsg = Serial.read();
  }
  if (distMsg == 'S') {
    motor(NOMVMT, NOMVMT);
  }
  else if (distMsg == 'F') {
    motor(SLOWMVMT, SLOWMVMT);
  }
  else {
    motor(FASTMVMT, FASTMVMT);
  }
  delay(50);
}
