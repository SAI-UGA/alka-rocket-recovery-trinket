///////////////////////////////////////////////////////////////////////////////////////
/*Terms of use
///////////////////////////////////////////////////////////////////////////////////////
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.


///////////////////////////////////////////////////////////////////////////////////////
//Support
///////////////////////////////////////////////////////////////////////////////////////
Website: http://www.brokking.net/imu.html
Youtube: https://youtu.be/4BoIE8YQwM8
Version: 1.0 (May 5, 2016)

///////////////////////////////////////////////////////////////////////////////////////
//Connections
///////////////////////////////////////////////////////////////////////////////////////
Power (5V) is provided to the Arduino pro mini by the FTDI programmer

Gyro - Arduino pro mini
VCC  -  5V
GND  -  GND
SDA  -  A4
SCL  -  A5

LCD  - Arduino pro mini
VCC  -  5V
GND  -  GND
SDA  -  A4
SCL  -  A5
*//////////////////////////////////////////////////////////////////////////////////////

//Include Servo and I2C library

#include <Wire.h>
#include <Servo.h>

//Declaring some global variables
int gyro_x, gyro_z, gyro_y;
long gyro_x_cal, gyro_z_cal, gyro_y_cal;
long loop_timer;
float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
boolean set_gyro_angles;
float angle_zero;
float angle_pitch_output, angle_roll_output;
int pitch_offset, roll_offset;

Servo servo;



void setup() {
  servo.attach(3);
  servo.write(0);
  Wire.begin();                                                        //Start I2C as master
 // Serial.begin(57600);                                               //Use only for debugging
  pinMode(13, OUTPUT);                                                 //Set output 13 (LED) as output
  
  setup_mpu_6050_registers();                                          //Setup the registers of the MPU-6050 (500dfs / +/-8g) and start the gyro

  digitalWrite(13, HIGH);                                              //Set digital output 13 high to indicate startup
  

 
                                                
  for (int cal_int = 0; cal_int < 2000 ; cal_int ++){                  //Run this code 2000 times
    read_mpu_6050_data();                                              //Read the raw acc and gyro data from the MPU-6050
    gyro_x_cal += gyro_x;                                              //Add the gyro x-axis offset to the gyro_x_cal variable
    gyro_z_cal += gyro_z;                                              //Add the gyro y-axis offset to the gyro_z_cal variable
    gyro_y_cal += gyro_y;                                              //Add the gyro z-axis offset to the gyro_y_cal variable
    delay(3);                                                          //Delay 3us to simulate the 250Hz program loop
  }
  gyro_x_cal /= 2000;                                                  //Divide the gyro_x_cal variable by 2000 to get the average offset
  gyro_z_cal /= 2000;                                                  //Divide the gyro_z_cal variable by 2000 to get the average offset
  gyro_y_cal /= 2000;                                                  //Divide the gyro_y_cal variable by 2000 to get the average offset
 
  digitalWrite(13, LOW);                                               //All done, turn the LED off
  
  loop_timer = micros();                                               //Reset the loop timer
}

void loop(){

  read_mpu_6050_data();                                                //Read the raw acc and gyro data from the MPU-6050

  gyro_x -= gyro_x_cal;                                                //Subtract the offset calibration value from the raw gyro_x value
  gyro_z -= gyro_z_cal;                                                //Subtract the offset calibration value from the raw gyro_z value
  gyro_y -= gyro_y_cal;                                                //Subtract the offset calibration value from the raw gyro_y value
  
  //Gyro angle calculations
  //0.0000611 = 1 / (250Hz / 65.5)
  angle_pitch += gyro_x * 0.0000611;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_z * 0.0000611;                                    //Calculate the traveled roll angle and add this to the angle_roll variable
  
  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_y * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_y * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel
  
  //Angle zero
  angle_zero = 90;
 
  
                                            //Drift compensation

  if(set_gyro_angles){                                                 //If the IMU is already started
    angle_pitch = angle_pitch * 0.9996 + angle_zero * 0.0004;     //Correct the drift of the gyro pitch angle with the set zero value (create artificial controlled drift)
    angle_roll = angle_roll * 0.9996 + angle_zero * 0.0004;        //Correct the drift of the gyro roll angle with the set zero value
  }
  else{                                                                //At first start
    angle_pitch = angle_zero;                                     //Set the gyro pitch angle equal to zero value wanted 
    angle_roll = angle_zero;                                       //Set the gyro roll angle equal to zero value wanted
    set_gyro_angles = true;                                            //Set the IMU started flag
  }
  
  //To dampen the pitch and roll angles a complementary filter is used
  angle_pitch_output = angle_pitch_output * 0.9 + angle_pitch * 0.1;   //Take 90% of the output pitch value and add 10% of the raw pitch value
  angle_roll_output = angle_roll_output * 0.9 + angle_roll * 0.1;      //Take 90% of the output roll value and add 10% of the raw roll value
  
  //activate servo
  deploy_chute(angle_pitch_output, angle_roll_output);


  while(micros() - loop_timer < 4000);                                 //Wait until the loop_timer reaches 4000us (250Hz) before starting the next loop
  loop_timer = micros();                                               //Reset the loop timer
}


void read_mpu_6050_data(){                                             //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x43);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,6);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 6);                                        //Wait until all the bytes are received
  gyro_x = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_x variable
  gyro_z = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_z variable
  gyro_y = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_y variable

}

void deploy_chute(double roll,double pitch){
  pitch = pitch + 90;                                                  //90 to adjust pith given values
  if(roll < 0 || pitch < 0){                                           // trigger when horizontal
  servo.write(90);
 // Serial.print("deploy");
  }
}

void setup_mpu_6050_registers(){
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission

  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
}
