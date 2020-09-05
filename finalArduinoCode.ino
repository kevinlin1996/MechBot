/*This code will be the set up for the slave Arduino running the
  LiDAR and motor controller*/
#include <mechbotShield.h>
#include <Wire.h>
#include <LIDARLite.h>
#include <LIDARLite_v3HP.h>

//General Constants
//NOTE: All constants need to be adjusted in the final design
#define ENCODERMIN 380    //This value is when the light sensor is covered
#define STEPANGLE 0.198
#define MAXSTEPS 50
#define DELAYAMOUNT 1000
#define STEPSPERREADING 18
#define INITANGLE -90
#define STOPDISTANCE 50
#define SLOWDISTANCE 100


//Port definitions
#define stp 11
#define dir 3
#define MS1 4
#define MS2 5
#define MS3 6
#define EN 7
#define BTN 13

LIDARLite lidarLite;
uint8_t calCnt = 0;
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
  Serial.begin(9600);
  lidarLite.begin(0, true);
  lidarLite.configure(0);

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

  digitalWrite(dir, LOW);
  motorDirection = -1;
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
    if (calCnt == 0) {
      lidarReading = lidarLite.distance();
    }
    else {
      lidarReading = lidarLite.distance(false);
    }

    //Increment counter
    calCnt++;
    calCnt = calCnt % 100;

    //Storing into array
    data[i] = lidarReading;

    //Calculating max distance in the x direction (forward on mechbot)
    currentDist = lidarReading;//abs(data[i] * cos((INITANGLE * motorDirection + STEPSPERREADING * motorDirection * i * STEPANGLE) * 3.14159 / (180.0)));

    //Compare to see if dist is closer than previous
    if ((currentDist < minDistance) && (data[i] > 20)) {
      if ((data[i] - data[i - 1]) < 50) {
        minDistance = currentDist;
      }
    }
  }
  
  //Serial.print(minDistance);
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
