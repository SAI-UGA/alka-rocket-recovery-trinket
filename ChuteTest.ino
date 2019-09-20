//look into changing the delay to millis

#include <TinyWireM.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_SoftServo.h>

Adafruit_SoftServo myServo1;
double maxHeight;
double currentHeight = 0;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

void setup() {
    OCR0A = 0xAF;
    TIMSK |= _BV(OCIE0A);
    myServo1.attach(1);
    myServo1.write(0);
    myServo1.write(90);
    TinyWireM.begin();
    //lcd.init();
    lcd.begin();
    lcd.backlight();
}

void loop() {
     //digitalWrite(2,HIGH);
     //chuteDeployTimer();
     lcd.setCursor(0,0);
     lcd.print("Hello World");
}


//Gets the time in seconds that the user want pin 2 to pulse at
void chuteDeployTimer() {
    //digitalWrite(1,HIGH);
    //myServo1.write(90);
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


//Reads the value of the altimeter
double readAltimeter() {
    double altitude = 0;
    baro.begin();
    double pascals = baro.getPressure();
    double logPressure = log(pascals/100000);
    double currentHeight = 3.2808*((-logPressure)*7700);
    setMaxHeight();
    return altitude;
  }

void setMaxHeight(){
    if (currentHeight < readAltimeter()){
        maxHeight = readAltimeter();
    }
}

//Displays the max height
void lcdDisplay() {
    boolean alt = true;
    while(alt){
        if (maxHeight - readAltimeter() <= 2){
            //display maxHeight
            lcd.backlight();
            lcd.setCursor(0,0);
            String str = String(maxHeight, 4);
            lcd.print("Max Height: " + str);
            alt = false;
        }
    }
  }

//Gets the veolocity of the Rocket
double checkVelocity() {
    double velocity = 0;
    double initalTime = millis()/1000;
    double deltaTime = (millis()/1000) - initalTime;
    double altimeterIntital = readAltimeter();
    velocity = (readAltimeter() - altimeterIntital)/deltaTime;
    return velocity;
  }

//Controls the chute deployment with Altimeter
void chuteDeployAltimeter(int time) {
    while(true) {
        if (checkVelocity() <= .0000000001) {
            readAltimeter();
            digitalWrite(2, HIGH);
            delay(time*1000);
            digitalWrite(2,LOW);
            lcdDisplay();
        } else {
            readAltimeter();
        }
    }

  }
