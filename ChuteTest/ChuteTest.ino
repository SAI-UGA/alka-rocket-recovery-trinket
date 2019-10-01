#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_MPL3115A2.h>
#include <Servo.h>
#include <MPU6050.h>

// Servo object to control the PWM signal
Servo myServo1;
double maxHeight = 0;
bool start = false;
double startVelocity = 10; // in ft/s
double currentHeight = 0;
double velocity = 0;
/** Base Height for our altimeter to get a more accurate max height 
 * (Athens, GA is 600 ft. above sea level, so we were getting 600 ft. on the ground)
 */
double baseHeight = 0;
double altimeterPrev = 0, altimeterCurrent = 0;
double timePrev = 0, timeCurrent = 0;
// Creating the display object or our OLED library
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, -1);
// Creating the barometer object for our altimeter library
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

// Boot setup
void setup()
{
    Serial.begin(9600);
    // Attaching the servo output to pin #1

    // starting the altimeter
    baro.begin();
    // Initialize the OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    // Turns on the display
    display.display();
    // Gets height of rocket before launch in feet
    baseHeight = 3.28084 * baro.getAltitude();
    altimeterCurrent=getCurrentHeight();
    timeCurrent=millis()/1000;
}

// Control loop for our Trinket
void loop()
{
  // Delay our loop by 20 ms to prevent the micropressor from locking up
  delay(20);

  // launch detection
  if (checkVelocity() > startVelocity)
  {
      start = true;
      myServo1.attach(3); // attaching the servo to digital pin 3
  }

  while (start)
  {
    // Delay our loop by 20 ms to prevent the microprocessor from locking up
    delay(20);

    // Updating the velocity to check if the rocket if post-apogee
    velocity = checkVelocity();

    // Logic that determine if the rocket is post-apogee (negative velocity acceleration)
    if (velocity < -10) // velocity is in ft/s
    {
      // activating the mechanical release system
      myServo1.write(0);
      myServo1.write(90);

      // writing our maxHeight to the display
      write(maxHeight);

      // traps the program in an infinite loop, essentially putting the nano to 'sleep'
      while(true)
      {
      }
    }
  }

}

//Reads the altimeter and sets the max height
void setMaxHeight(double currentHeight)
{
  // Sets the new max height
  if (currentHeight > maxHeight)
  {
    maxHeight = currentHeight;
  }
}

// Returns the current height of our rocket by reading the altimeter
double getCurrentHeight()
{
  // reading the altimeter in meters
  double altm = baro.getAltitude();
  // meters to feet conversion
  currentHeight = 3.28084 * altm;
  //return the change in height in feet
  return currentHeight - baseHeight;
}

//Displays the max height
void write(double maxHeight)
{
    // Clears the display
    display.clearDisplay();
    // Sets the text size
    display.setTextSize(1);
    // Sets the text color
    display.setTextColor(WHITE);  // I dont think we need this
    // Set cursor position 1
    display.setCursor(0,0);
    // Prints to display
    display.print("Max Altitude");
    // Sets cursor for new line
    display.setCursor(0,8);
    // Prints altitude
    display.print(String(maxHeight));
    // Turns on display
    display.display();
}

// // Gets the rocket velocity
// double checkVelocity() {
//     // Sets previous height to the current height
//     altimeterPrev = altimeterCurrent;
//     // Sets the previous time to the current time
//     timePrev=timeCurrent;
//     // Gets the current height
//     altimeterCurrent = getCurrentHeight();
//     // Gets the current time
//     timeCurrent=(double)millis()/1000;
//     // Updates max height
//     if(altimeterCurrent>altimeterPrev) {
//         setMaxHeight(altimeterCurrent);
//     }
//     // Finds the change in time
//     double delta = timeCurrent-timePrev;
//     // Calculates velocity if change in time is greater than 0;
//     double velocity =  (altimeterCurrent-altimeterPrev)/(delta>0?delta:20);
//     return velocity;

    double checkVelocity() {

    }
}
