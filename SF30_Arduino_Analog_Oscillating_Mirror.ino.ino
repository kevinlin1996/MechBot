/*This code is to validate the viability of
   using 2 markers (obstacles) in front of the
   mirror/lidar combination to see if
   we can use this interpolate the angle corresponding
   to the lidar reading using known values
*/

//clear and set bit functions for changing ADC values
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

//Constants
#define BAUDRATE 115200
#define BETA 45.0 //Refer to diagram, corresponds to fixed incident angle (in degrees) of laser to mirror (horizontal)
#define MAXRANGE 5.0 //max. set range for LIDAR (in metres)
#define MAXVOLTAGE 2.048 //max. analog voltage output (in Volts)
#define SAMPLEFREQ 150 //Number of samples per sweep
#define CALIDIST 0.25 //calibration distance of 2 objects in LIDAR sweep
#define ALPHA1 -10 //Angle of mirror at lower limit position (in degrees)
#define ALPHA2 10 //Angle of mirror at upper limit position (in degrees)


//Pins
#define LIDARANALOG 3 //pin to read analog from


//Global Variables
float phi1; //lower limit angle of lidar sweep
float phi2; //upper limit angle of lidar sweep
float phi10;  //current angle WRT base
float phi; //angle of laser WRT to the phi1 (phi10 - phi1)
float anglePeriod;  //Period of time for mirror to rotate between smallest increment of angle
float readingAng[SAMPLEFREQ]; //Array of each angle
float readingValue[SAMPLEFREQ]; //Array of each reading
boolean dir = 0; //directions of mirror oscillation. 0 = towards lower limit, 1 = towards upper limit
int reading;
float lidarReading;
float oldtime = 0.0;
int i = 0;

void setup() {
  
  //max reading rate for atmega based boards is 10kHz
  Serial.begin(BAUDRATE);

  //Change prescale of the ADC clock to 16 (default is 128)
  //ADC clock freq = 16 MHz/Prescale
  //One conversion takes 13 ADC clocks
  //Default sample rate = ADC clock freq/13
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  boolean set = 0;  //Used for the initial setup of angle calibration
  float initdist[5];  //array used to look for calibration
  float distsum = 0.0;
  float distance;
  float lowerTime, upperTime, mirrorPeriod; //is float large enough data type?
  int i;


  //Lower limit of LIDAR sweep (angle in degrees)
  phi1 = BETA + 2 * (90 - BETA + ALPHA1);
  //upper limit of lidar sweep
  phi2 = BETA + 2 * (90 - BETA + ALPHA2);

  //Initial calibration
  while (!set) {
    
    distsum = 0;

    for (i = 0; i < 5; i++) {
      //take a reading
      distance = readDistance();
      initdist[i] = distance;
      distsum = distsum + initdist[i];
    }

    //Measured lower limit reference (0-20 cm)
    if ((distsum > 0) && (distsum < 1.00)) {
      Serial.println("lower reference found");
      

      lowerTime = micros();
    

      while (!set) {
        
        distsum = 0;

        for (i = 0; i < 5; i++) {

          //take a reading
          distance = readDistance();
          initdist[i] = distance;
          distsum = distsum + initdist[i];
        }

        //Measured upper limit reference (20-30 cm)
        if ((distsum > 1.00) && (distsum < 1.50)) {
          
          upperTime = micros();
          Serial.println("upper reference found");


          mirrorPeriod = upperTime - lowerTime; //half the full period of mirror oscillation
          anglePeriod = mirrorPeriod / SAMPLEFREQ;
          set = 1;
          
          Serial.println(lowerTime);
          Serial.println(upperTime);



        }


      }
    }


  }

}


//Function to read distance and return it in metres
float readDistance() {
  float voltage = analogRead(LIDARANALOG) * (5.0 / 1024.0);     //map input voltage between 0-5V to integer value between 0-1023 (10 bit resolution)
  float distance = (voltage / MAXVOLTAGE) * MAXRANGE;
  return distance;
}


//Void loop starts immediately after upper limit is read and mirror period is calculated in microseconds
void loop() {

  //moving towards lower limit
  if (dir == 0) {

    //Take a reading every angle period
    if ( ((micros() - oldtime) > anglePeriod) || oldtime == 0) {
      oldtime = micros();


      if (i < SAMPLEFREQ) {


 
        reading = SAMPLEFREQ - i;
        readingValue[reading] = readDistance();
        readingAng[reading] = phi2 - i * ((phi2 - phi1) / SAMPLEFREQ); //would have to adjust if want WRT phi1
        Serial.print(readingAng[i]);
        Serial.print(" ");
        Serial.println(readingValue[i]);
        i++;
                 Serial.println("hi");

      }

      else if (i == SAMPLEFREQ) {
        i = 0;
        dir = 1;
      }

    }

  }


  //moving towards upper limit
  else if (dir == 1) {

    //Take a reading every angle period
    if ( ((micros() - oldtime) > anglePeriod) || oldtime == 0) {
      oldtime = micros();

      if (i < SAMPLEFREQ) {

        reading = i;
        readingValue[reading] = readDistance();
        readingAng[reading] = phi1 + i * ((phi2 - phi1) / SAMPLEFREQ);
        Serial.print(readingAng[i]);
        Serial.print(" ");
        Serial.println(readingValue[i]);
        i++;
      }

      else if (i == SAMPLEFREQ) {
        i = 0;
        dir = 0;
      }

    }
  }


}
