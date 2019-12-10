
/*/////////////////////////////////////////////////////////////////////////////////////
This code was based on the code from http://www.brokking.net/imu.html
///////////////////////////////////////////////////////////////////////////////////////
//Connections
///////////////////////////////////////////////////////////////////////////////////////
Power (5V) is provided to the Arduino pro mini by the FTDI programmer

Gyro - MPU6050
VCC  -  5V
GND  -  GND
SDA  -  A4
SCL  -  A5

LCD  - Arduino nano
VCC  -  5V
GND  -  GND
SDA  -  A4
SCL  -  A5
*//////////////////////////////////////////////////////////////////////////////////////

//Include Servo and I2C library

#include <Wire.h>
#include <Servo.h>

#define DeployDelay 10000          	                                  //enter delay after launch (in ms)
#define LaunchAcc 3                                                    //enter acceleration at launch (in G)

//Declaring some global variables
int gyro_x, gyro_z, gyro_y;
long acc_x, acc_z, acc_y;
int temperature;
int launch_delay;
long launch_acc;
long gyro_x_cal, gyro_z_cal, gyro_y_cal;
long acc_y_cal;
long loop_timer;
long t_o_l;
float angle_pitch, angle_roll;
boolean has_launched;

Servo servo;



void setup() {
  servo.attach(5);                                                     //attach servo to pin 3
  servo.write(0);                                                      //initialize the servo starting angle
  Wire.begin();                                                        //Start I2C as master
  Serial.begin(57600);                                                 //Use only for debugging
  pinMode(13, OUTPUT);                                                 //Set output 13 (LED) as output
  
  setup_mpu_6050_registers();                                          //Setup the registers of the MPU-6050 (500dfs / +/-16g) and start the gyro

  digitalWrite(13, HIGH);                                              //Set digital output 13 high to indicate startup
  
                                          
  for (int cal_int = 0; cal_int < 2000 ; cal_int ++){                  //Run this code 2000 times
    read_mpu_6050_data();                                              //Read the raw acc and gyro data from the MPU-6050
    gyro_x_cal += gyro_x;                                              //Add the gyro x-axis offset to the gyro_x_cal variable
    gyro_z_cal += gyro_z;                                              //Add the gyro y-axis offset to the gyro_z_cal variable
    gyro_y_cal += gyro_y;                                              //Add the gyro z-axis offset to the gyro_y_cal variable
    acc_y_cal += acc_y;												   	                      //Add the acc y-axis offset to the acc_y_cal variable
    delay(3);                                                          //Delay 3us to simulate the 250Hz program loop
  }
  gyro_x_cal /= 2000;                                                  //Divide the gyro_x_cal variable by 2000 to get the average offset
  gyro_z_cal /= 2000;                                                  //Divide the gyro_z_cal variable by 2000 to get the average offset
  gyro_y_cal /= 2000;                                                  //Divide the gyro_y_cal variable by 2000 to get the average offset
  acc_y_cal /= 2000;                                                   //Divide the acc_y_cal variable by 2000 to get the average offset

  angle_pitch = 0;                                                     //set starting angle at 0
  angle_roll = 0;                                                      //set starting angle at 0

  digitalWrite(13, LOW);                                               //All done, turn the LED off
  
  loop_timer = micros();                                               //Reset the loop timer
}

void loop(){

  read_mpu_6050_data();                                                //Read the raw acc and gyro data from the MPU-6050
  

  gyro_x -= gyro_x_cal;                                                //Subtract the offset calibration value from the raw gyro_x value
  gyro_z -= gyro_z_cal;                                                //Subtract the offset calibration value from the raw gyro_z value
  gyro_y -= gyro_y_cal;                                                //Subtract the offset calibration value from the raw gyro_y value
  acc_y -= acc_y_cal;                                                  //Subtract the offset calibration value from the raw acc_y value
  //Serial.println(acc_y);
  
  //Gyro angle calculations
  //0.0000611 = 1 / (250Hz / 65.5)
  angle_pitch += -gyro_x * 0.0000611;                                  //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_z * 0.0000611;                                    //Calculate the traveled roll angle and add this to the angle_roll variable
  
  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_y * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_y * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel
  

  // both ways to activate the release
  timer_deploy(acc_y);
  deploy_chute(angle_pitch, angle_roll);

  while(micros() - loop_timer < 4000);                                 //Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
  loop_timer = micros();                                               //Reset the loop timer
}


void deploy_chute(double roll,double pitch){ 
  if(abs(roll) > 90 || abs(pitch) > 90){                               // trigger when horizontal
  servo.write(90);
  delay(500);
  digitalWrite(13, HIGH);											   //when trigger, turn on LED (for troubleshooting)
  while(true);
  }
}


void timer_deploy(long acc){
  if((abs(acc) > (LaunchAcc*2048)) && has_launched == false){			   //detect launch, start counter
    has_launched = true;
    t_o_l = millis();
  }
  Serial.println(millis() - t_o_l);
   
  if(has_launched && ((millis() - t_o_l) > DeployDelay)){			   //after deploy delay, activate servo
    servo.write(90);
    delay(500);	
    while(true);
  }
}


void read_mpu_6050_data(){                                             //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                        //Wait until all the bytes are received
  acc_x = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_x variable
  acc_y = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_z variable
  acc_z = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_y variable
  temperature = Wire.read()<<8|Wire.read();                            //Add the low and high byte to the temperature variable
  gyro_x = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_x variable
  gyro_y = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_z variable
  gyro_z = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_y variable
}


void setup_mpu_6050_registers(){
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0b00011000);                                              //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
}
