/*This code will be the set up for the slave Arduino running the
  LiDAR and motor controller*/
#include <mechbotShield.h>
#include <Wire.h>
#include <LIDARLite.h>
#include <LIDARLite_v3HP.h>

//General Constants
//NOTE: All constants need to be adjusted in the final design
#define ENCODERMIN 390    	//This value is when the light sensor is covered
#define STEPANGLE 0.198		
#define MAXSTEPS 50			//MAXSTEPS * STEPSPERREADING * STEPANGLE = ABS(2*INITANGLE) 
#define DELAYAMOUNT 800
#define STEPSPERREADING 9
#define INITANGLE -45
#define STOPDISTANCE 40
#define SLOWDISTANCE 70


//Port definitions for motor control
#define stp 11
#define dir 3
#define MS1 4
#define MS2 5
#define MS3 6
#define EN 7
#define BTN 13

LIDARLite_v3HP lidarLite;
int data[MAXSTEPS];
int8_t motorDirection = 1; //-1: dir = LOW, 1: dir = HIGH
float minDistance;

void setup() {
  int zeroSensor;

  //Motor
  pinMode(stp, OUTPUT);   //HIGH will make the motor step once
  pinMode(dir, OUTPUT);   //HIGH -> "reverse" or CCW, LOW -> "forward" or CW
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(EN, OUTPUT);
  
  digitalWrite(EN, LOW);  //Pull low to set FETs active to allow for motor control. Otherwise HIGH to lock
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(MS3, LOW);

  initADC();

  //LiDAR Camera
  Wire.begin();
  if (true) {
#if ARDUINO >= 157
    Wire.setClock(400000UL);
#else
    TWBR = ((F_CPU / 400000UL) - 16) / 2;
#endif
  }
  lidarLite.configure(0);

  Serial.begin(19200);
  //Setting up Photoresistors for reading
  digitalWrite(dir, HIGH);

  do {
    //Run motor towards zero
    digitalWrite(stp, LOW);
    delay(5);
    digitalWrite(stp, HIGH);
    delay(5);
    zeroSensor = analog(0);
  } while (zeroSensor > ENCODERMIN);
  
  //Code to set motor to start at 45 deg.
  digitalWrite(dir, LOW);
  motorDirection = -1;
  int offset = 0;
  do {
    offset++;
    //Run motor towards a 45 degree start
    digitalWrite(stp, LOW);
    delay(5);
    digitalWrite(stp, HIGH);
    delay(5);
  } while (offset < 227);
}

void loop() {
  int lidarReading;
  float currentDist;

  minDistance = 1000;

  for (int i = 0; i < MAXSTEPS; i++) {
    //Step Motor
    for (int j = 0; j < STEPSPERREADING; j++) {
      digitalWrite(stp, HIGH);
      delayMicroseconds(DELAYAMOUNT);
      digitalWrite(stp, LOW);
      delayMicroseconds(DELAYAMOUNT);
    }

    //LiDAR Reading
    lidarLite.takeRange();
    lidarReading = lidarLite.readDistance();

    //Storing into array
    data[i] = lidarReading;

    //Calculating max distance in the x direction (forward on mechbot)
    currentDist = abs(data[i] * cos((INITANGLE * motorDirection + STEPSPERREADING * motorDirection * i * STEPANGLE) * 3.14159 / (180.0)));

    //Compare to see if dist is closer than previous
    if ((currentDist < minDistance) && (data[i] > 15)) {	//Check for the closest distance and if it is above the optimal reading number
      if ((data[i] - data[i - 1]) < 50) {		//Check to see if a random reading is present
        minDistance = currentDist;
      }
    }
  }
  //Printing to Serial port
  if (minDistance < STOPDISTANCE) {
    Serial.write('S');
  }
  else if (minDistance < SLOWDISTANCE) {
    Serial.write('F');
  }
  else {
    Serial.write('G');
  }

  //Switching motor direction
  motorDirection = motorDirection * -1;
  switch (motorDirection) {
    case 1:
      digitalWrite(dir, HIGH);
      delay(1);
      break;
    case-1:
      digitalWrite(dir, LOW);
      delay(1);
      break;
  }
}
