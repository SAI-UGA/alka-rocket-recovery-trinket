#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_MPL3115A2.h>
#include <Servo.h>

Servo myServo1;
double maxHeight = 0;
double currentHeight = 0;
double velocity = 0;
double baseHeight = 0;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire, -1);
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

// Boot setup
void setup()
{
    // Attaching the servo output to pin #1
    myServo1.attach(3);
    // starting the altimeter
    baro.begin();
    // Initialize the OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    // Turns on the display
    display.display();
    delay(1000);
    // Gets height of rocket before launch in feet
    baseHeight = 3.28084 * baro.getAltitude();
}

// Control loop for our Trinket
void loop()
{
  // launch detection
  bool start = false;
  double startVelocity = 3.28084; // in ft/s (1 m/s)
  if (checkVelocity() >= startVelocity)
  {
    start = true;
  }

  while (start)
  {
    velocity = checkVelocity();
    if (velocity < 0)
    {
      // activating the mechanical release system
      for (int pos = -180; pos <= 18; pos ++) {
          myServo1.write(pos);
      }
      // writing our maxHeight to the display
      write(maxHeight);
      // traps the program in an infinite loop, essentially putting the trinket to 'sleep'
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

//Gets the velocity of the Rocket (in ft/ms)
double checkVelocity()
{
  double velocity = 0;
  //gets the height in feet
  double altimeterIntital = getCurrentHeight();
  //gets the current time milliseconds
  double initalTime = millis()/1000;
  //find the change in time between the last mills call
  double deltaTime = (millis()/1000) - initalTime;
  //calculates the "instintanious" rate of change of the rocket
  velocity = (getCurrentHeight() - altimeterIntital)/deltaTime;
  velocity = velocity / 1000; // converting the velocity to ft/s
  setMaxHeight(getCurrentHeight());
  return velocity;
}

// volatile uint8_t counter = 0;
// SIGNAL(TIMER0_COMPA_vect) {
//   // this gets called every 2 milliseconds
//   counter += 2;
//   // every 20 milliseconds, refresh the servos!
//   if (counter >= 20) {
//     counter = 0;
//     myServo1.refresh();
//   }
// }
