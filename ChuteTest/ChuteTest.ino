//look into changing the delay to millis

#include <TinyWireM.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_SoftServo.h>

Adafruit_SoftServo myServo1;
double maxHeight = 0;
double currentHeight = 0;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

// First time set up
void setup() 
{
    OCR0A = 0xAF;
    TIMSK |= _BV(OCIE0A);

    // Attaching the servo output to pin #1
    myServo1.attach(1);
    // Begin TinyWireM to use I2C
    TinyWireM.begin();
    // LCD setup (changing to OLED soon)
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0,0);
}

// Control loop for our Trinket
void loop() 
{

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
  // starting the altimeter
  baro.begin();
  // reading the altimeter in meters
  double altm = baro.getAltitude();
  // meters to feet conversion
  currentHeight = 3.28084 * altm;

  return currentHeight;
}

//Displays the max height
void write(double maxHeight) 
{
  lcd.write("Max Height:");
  lcd.setCursor(0,1);
  string str = String(maxHeight) + " ft";
  lcd.write(str);
}

// Gets the veolocity of the Rocket
double checkVelocity() 
{
  double velocity = 0;
  double initalTime = millis()/1000;
  double deltaTime = (millis()/1000) - initalTime;
  double altimeterIntital = getCurrentHeight();
  velocity = (getCurrentHeight() - altimeterIntital)/deltaTime;
  setMaxHeight(getCurrentHeight());
  return velocity;
}

volatile uint8_t counter = 0;
SIGNAL(TIMER0_COMPA_vect) {
  // this gets called every 2 milliseconds
  counter += 2;
  // every 20 milliseconds, refresh the servos!
  if (counter >= 20) {
    counter = 0;
    myServo1.refresh();
  }
}