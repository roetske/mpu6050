/*
References:
    . MadgwickAHRS Filter Algorithm
          http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/
    . Jeff Rowberg's MPU6050
          https://github.com/jrowberg/i2cdevlib
    . Kris Winer's MPU6050
          https://github.com/kriswiner/MPU6050
    . Davide Gironi's AVR atmega MPU6050
          http://davidegironi.blogspot.com/2013/02/avr-atmega-mpu6050-gyroscope-and.html#.W7zM7mgzaUk          
  
  modules
  --------
  esp32 dev mod 38 pins
  lcd i2c 20x4 
  mpu6050 i2c 
  levelshifter 5 3.3 to 5v (needed for lcd does not work on 3.3v (backlight))
  green led active blink
  pushbutton for calibration
  
  oef da werkt en stabiel =>setting clock naar ref acc maakt deze stabiel
  bij pitch van 90° verspringen roll en ptich met 180° dit is blijkbaar normaal 
  calibratie met drukknop 
         opgelet geen serial com in meting anders shit vertraging
  store offsetts in flash na kalibreren
         bij herstart zal deze de nw offsets laden van flash
  yaw is niet stabiel verloopt je hebt hier magnetometer voor nodig => bye bye yaw for this sketch
  hoeken 1 cijfer na comma
  debug zit  max aantal keren schrijven anders flash direct stuk :)(knop blijft hangen stel je voor blijft schrijven) 
*/

//---------------------------------------------------------------------------------------------------
//includes
#include<Wire.h>
#include <math.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include "definitions_mpu6050.h"
#include <Preferences.h>
//preferences use flash read write 
Preferences preferences;
//Initialize the LCD library
LiquidCrystal_I2C lcd(0x27,20,4);

// For Quaternion function
#define twoKpDef  (2.0f * 0.5f) // 2 * proportional gain
#define twoKiDef  (2.0f * 0.0f) // 2 * integral gain

// change these parameters for gyro and acc according to your choice (see pdf mpu6050) 
//settings in MPU6050_Init() 2000deg/s 8g
//acc
/////
#define MPU6050_AccGAIN 4096.0 // AFS_SEL = 2, +/-8g, MPU6050_ACCEL_FS_8
//Gyro
//////
#define MPU6050_GyroGAIN 16.384 // FS_SEL = 3, +/-2000degree/s, MPU6050_GYRO_FS_2000

// Blinking LED
#define LED_PIN 27
#define sda 21
#define scl 22
#define pushbutton 32
//---------------------------------------------------------------------------------------------------
// Var
//Version
String version ="Version 6"; //van mijn collega overgenomen via serial welke versie belangrijk
//calculations
volatile float twoKp = twoKpDef;                      // 2 * proportional gain (Kp)
volatile float twoKi = twoKiDef;                      // 2 * integral gain (Ki)
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;          // quaternion of sensor frame relative to auxiliary frame
volatile float integralFBx = 0.0f,  integralFBy = 0.0f, integralFBz = 0.0f; // integral error terms scaled by Ki
//offsets are loaded from flash or when calibrating
float MPU6050_AXOFFSET = 0;
float MPU6050_AYOFFSET = 0;
float MPU6050_AZOFFSET = 0;
float MPU6050_GXOFFSET = 0;
float MPU6050_GYOFFSET = 0;
float MPU6050_GZOFFSET = 0;

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float axg, ayg, azg, gxrs, gyrs, gzrs;
float roll, pitch, yaw;
float GyX_cal ,GyY_cal ,GyZ_cal ,AcX_cal ,AcY_cal,AcZ_cal ;

float SelfTest[6];
//used in complicated calibration
float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0}; // Bias corrections for gyro and accelerometer

//for blinking led
bool blinkState = false; // LED Bliking for Pin13

bool bcalibrate;
bool ballowcalibration;
//timechecks
elapsedMillis tlcd;
elapsedMillis tmpu;
elapsedMillis tcycle;
elapsedMillis tblinkled;
elapsedMillis treadpushbutton;
elapsedMillis tallowcalibration;
int intervalcalibration = 5000;
int intervalpushbuttonread =100 ;
int intervalblinkled = 200;
int DTmpu = 20; //tijd update mpu
float DTs = (DTmpu/1000.0); //millis to seconds hier moeten aanpassen aan millis elapsedmillis dt tijd nodig berekenen
int intervallcd = 300;

//debug write flash
int countwrite =0;
int maxwrite = 10;

void setup() {
  Wire.begin(sda,scl); // set pin comm i2c
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
  Serial.begin(115200);
 
  // Self Test
  MPU6050SelfTest(SelfTest);
  //gyro 2000deg/s  acc 8g
  MPU6050_Init();
  // Configure LED for output
  pinMode(LED_PIN, OUTPUT);
  pinMode(pushbutton,INPUT);
  initlcd(); 
  //checkstored calib
  readFlashOffsetcal();  
  //allow calibration when pushbutton pressed
  ballowcalibration = true;
}

void loop() {
  //main
  if ((tmpu >DTmpu) and !bcalibrate )
    {  
        DTs =  tmpu/1000.0; //compute with real time
		mpu6050_GetData();
        // Update raw data to Quaternion form
		mpu6050_updateQuaternion();
    //compute data
    MahonyAHRSupdateIMU(gxrs, gyrs, gzrs, axg, ayg, azg);
    // Value of Roll, Pitch, Yaw
    mpu6050_getRollPitchYaw();
		tmpu =0;
    }
  //pushbutton calibrate
  if (treadpushbutton > intervalpushbuttonread)
   { if (digitalRead(pushbutton) and ballowcalibration)
       { ballowcalibration = false;
         bcalibrate = true;
         docalibration();
         bcalibrate = false;
         //allow only next cycle to measure
         tmpu =0;
         }
      treadpushbutton= 0;
   }
  if ((tallowcalibration >intervalcalibration) and  !bcalibrate )
      { 
        ballowcalibration = true;
        tallowcalibration = 0;
      } 
   //lcd
   if (tlcd > intervallcd)
    {  if (!bcalibrate)
         { //write_LCD();
          char  chpitch[7];
          char  chroll[7];
          dtostrf(roll, 0, 1, chroll);
          dtostrf(pitch, 0, 1, chpitch);      
          lcd.setCursor(6,0);
          lcd.print(chpitch);        
          lcd.setCursor(6,1);
          lcd.print(chroll);
         }          
       if (bcalibrate and ballowcalibration)
         { //write_LCD();
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Need Calibration");
          }
     tlcd=0;     
    }
  //check dutytime via serial dt cycle moet steeds kloppen
  if (tcycle > DTmpu ) 
    { Serial.print(">");
      Serial.print( DTmpu);
      Serial.println("ms");      
      Serial.println(tcycle);
    }  
  if (tblinkled > intervalblinkled)
    {
     // Blink LED to indicate activity
     blinkState = !blinkState;
     digitalWrite(LED_PIN, blinkState);
     tblinkled = 0;
    }
  tcycle = 0;
}

void initlcd()
{
  lcd.begin();                //Initialize the LCD
  lcd.backlight();            //Activate backlight
  lcd.clear();                //Clear the LCD
  lcd.setCursor(0,0);         //Set the LCD cursor column,row
  lcd.print("  MPU-6050 IMU");                                         
  lcd.setCursor(0,1);                                                 
  lcd.print(version);                                             
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);    
  lcd.print("dtms=");
  lcd.print(DTmpu);
  lcd.setCursor(0,1); 
  lcd.print("Gyrogain=");
  lcd.print( MPU6050_GyroGAIN);
  lcd.setCursor(0,2); 
  lcd.print("AccGain=");
  lcd.print( MPU6050_AccGAIN,8);
  delay(5000);                                                        
  lcd.clear();   
}



