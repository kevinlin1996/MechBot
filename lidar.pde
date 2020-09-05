import processing.serial.*;

Serial mySerial;        // The serial port

int iter = 0;
float x = 500;
float y = 500;

void setup () {
  size(1000, 1000); // set the window size:  
  println(Serial.list());
  String myPort = Serial.list() [0]; // Find the correct Serial Port
  mySerial = new Serial(this, myPort, 9600);  //Open port
  println("Check1");
  // A serialEvent() is generated when a newline character is received :
  mySerial.bufferUntil('\n');
  background(0);      //set inital background
  println("Check2");
}

void draw () {
  //Drawing a line from center to point.
  stroke(255, 0, 0);     //stroke color (red, green, blue)
  line( width/2, height/2, x, y);    //line from center of the screen to x and y
  stroke(250); //different color brush
  point(x, y); //draw point
}

void serialEvent (Serial mySerial) {
  // get the ASCII string:
  String inString = mySerial.readStringUntil('\n');  //reads the string until there is a new line
  // Skip first iteration 
  if (iter == 1) {
    String splitted[] = split(inString, ' ');    //splits string whenever there is a space
    String inString1 = splitted[0];    //first half of split
    String inString2 = splitted[1];    //second half of split
    if (inString1 !=  null || inString2 != null || inString1 != " " || inString2 != " ") {
      inString1 = trim(inString1);                // trim off whitespaces.
      inString2 = trim(inString2); 
      float dist = float(inString1);           // convert to a number.
      float angl = float(inString2);
      if (!Float.isNaN(dist) || !Float.isNaN(angl)) { 
        dist = map(dist, 0, 500, 0, width);    //maps the variable dist from 0 to 4 and then into a value between 0 and the width of the window
       
        x = width/2 + dist*sin(angl);
        y = height/2 + dist*cos(angl);
      }
    }
  }
  println(mySerial.readStringUntil('\n'));
  iter = 1;
}
