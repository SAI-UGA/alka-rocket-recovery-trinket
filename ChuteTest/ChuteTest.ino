#include <Wire.h>
#include <Servo.h>

// Indicator Light
#define light 4;

Servo servo;

// Sets the gyro address to a variable
const int MPU_addr=0x68;

// Sets variables for the acceleration values;
int16_t AcX,AcY,AcZ;

// Sets valiables for the min and max values for mapping
int minVal=265;
int maxVal=402;

// our x, y, and z axis angles
double x;
double y;
double z;

//Initial orientation values
double xInitial;
double yInitial;
double zInitial;


// Boot setup
void setup() {
    // Indicate Nano on
    digitalWrite(light, HIGH);
    Wire.begin();
    // Begin talking to Gyro at address 0x68
    Wire.beginTransmission(MPU_addr);
    // Sets the register to be acceseed
    Wire.write(0x6B);
    // Writes the 0x6B register to 0 setting clock to 8MHz
    Wire.write(0);
    Wire.endTransmission(true);
    Serial.begin(9600);
    // Calls function to set initial orientation
    setBase();
    //Serial.println("Done");
}

// Control loop for our Arduino nano
void loop() {
  // Reads the gyro
  readGyro();
  // If the Gyro detects a orientation change of 90 degrees of more deploy chute
  // Serial.println("xInitial: " + String(xInitial));
  // Serial.println("zInitial: " + String(zInitial));
  if(abs(x-xInitial) >= 270 || abs(x-xInitial) >= 85
  || abs(z-zInitial) >= 270 || abs(z-zInitial) >= 85 ) {
      servo.attach(3);
      // Serial.println("DEPLOY");
      servo.write(180);
      while(true){

      }
  } else {
      // Serial.println("NOT DEPLOYED");
      servo.write(0);
  }
}

void readGyro() {
    // begin transmiting to the Gyro
    Wire.beginTransmission(MPU_addr);
    // Sets the register to be accessed
    Wire.write(0x3B);
    // Keeps the bus open
    Wire.endTransmission(false);
    // Request the registries for orientation information (device, number of registries, strop when done)
    Wire.requestFrom(MPU_addr,14,true); // default (MPU_addr,14,true)
    // Gyro registers store 8bit values, but takes 16 bit readings
    AcX=Wire.read()<<8|Wire.read();
    AcY=Wire.read()<<8|Wire.read();
    AcZ=Wire.read()<<8|Wire.read();
    // scales the Gyro data to angle values between -90 and 90
    int xAng = map(AcX,minVal,maxVal,-90,90);
    int yAng = map(AcY,minVal,maxVal,-90,90);
    int zAng = map(AcZ,minVal,maxVal,-90,90);
    //Converts the mapped numbers to usable anble measures in degrees
    x= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI);
    y= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI);
    z= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
    //
    // Serial.println("xAngle: " + String(x));
    // Serial.println("yAngle: " + String(y));
    // Serial.println("zAngle: " + String(z));

    //delay(200);
}
// Gets the average value of the x and z angles
void setBase() {
    //  Sets values to sum the x and z readings
    int xAv = 0;
    int zAv = 0;
    // Sets the number of values to read
    int numValues = 100;
    for (int start = 0; start < numValues; start++) {
        readGyro();
        xAv = xAv + x;
        zAv = zAv + z;
    }
    // Takes the average of the values
    xInitial = xAv/numValues;
    zInitial = zAv/numValues;
}
