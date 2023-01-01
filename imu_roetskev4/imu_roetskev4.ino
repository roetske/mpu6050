
/*Terms of use
TWARE.


///////////////////////////////////////////////////////////////////////////////////////
//Support based on version from joop brokken
///////////////////////////////////////////////////////////////////////////////////////
Website: http://www.brokking.net/imu.html
Youtube: https://youtu.be/4BoIE8YQwM8
Version: 1.0 (May 5, 2016)

///////////////////////////////////////////////////////////////////////////////////////
//Connections
///////////////////////////////////////////////////////////////////////////////////////
esp32 dev mod
levelshifter
i2c 
sda gpio21 
scl pgio22
led 27

0x68 mpu6050
0x27 lcd

Opmerkingen
calibratie nok xgyro 64000? 
    oplossing  int16_t ipv long
      int16_t gyro_x, gyro_y, gyro_z;
      int16_t acc_x, acc_y, acc_z      
berekeningen dt aanpasbaar te maken ok
rekening bij houden calibreren met button
waardes opslaan als processor herstart om gelijk welke reden zal die opnieuw kalibreren
*//////////////////////////////////////////////////////////////////////////////////////

//Include LCD and I2C library
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <elapsedMillis.h>
//Initialize the LCD library
LiquidCrystal_I2C lcd(0x27,20,4);

#define Led 27
#define sda 21
#define scl 22


//Declaring some global variables
int16_t gyro_x, gyro_y, gyro_z;
int16_t acc_x, acc_y, acc_z;
int16_t temperature;

long gyro_x_cal, gyro_y_cal, gyro_z_cal;
long acc_x_cal, acc_y_cal, acc_z_cal;
float acc_total_vector;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch_output, angle_roll_output;
float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
int count;
String version ="Version 4";

bool set_gyro_angles;
bool blevel;
bool bcalibrate;
int lcd_loop_counter;

elapsedMillis tlcd;
elapsedMillis tmpu;
elapsedMillis tcycle;
int DTmpu = 20;
float DTs = (DTmpu/1000.0); //micros to seconds
int intervallcd =30;


float gyroset250 =  (float) DTs/131.0;//0.0000305344;
float gyroset500 =  (float) DTs/65.5;//0.0000610687 4ms
float gyroset1000 = (float) DTs/32.8;//0.0001219512
float gyroset2000 = (float) DTs/16.4;//0.0002439024
float gyrogain;
float gyrogainRad;
float correctpitch =0;
float correctroll =0;
//#define DEG_TO_RAD 0.017453292519943295769236907684886
//#define RAD_TO_DEG 57.295779513082320876798154814105

void setup()
 {
  Wire.begin(sda,scl); // set pin comm i2c
  Wire.setClock(400000);//i2c speed                                    //Start I2C as master
  
  
  Serial.begin(115200);                                               //Use only for debugging
   while (!Serial)
    {
    delay(10); // will pause 
    }
                                           
  
  setup_mpu_6050_registers();                                          //Setup the registers of the MPU-6050 (500dfs / +/-8g) and start the gyro 
  gyrogain = gyroset500;
  gyrogainRad = (float) gyroset500*DEG_TO_RAD;
  
  lcd.begin();                                                         //Initialize the LCD
  lcd.backlight();                                                     //Activate backlight
  lcd.clear();                                                         //Clear the LCD

  lcd.setCursor(0,0);                                                  //Set the LCD cursor to position to position 0,0
  lcd.print("  MPU-6050 IMU");                                         //Print text to screen
  lcd.setCursor(0,1);                                                  //Set the LCD cursor to position to position 0,1
  lcd.print(version);                                              //Print text to screen
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);    
  lcd.print("dtms=");
  lcd.print(DTmpu);
  lcd.setCursor(0,1); 
  lcd.print("gyrogain=");
  lcd.print( gyrogain,8);
  lcd.setCursor(0,2); 
  lcd.print("gainRad=");
  lcd.print( gyrogainRad,8);
  delay(5000);                                                         //Delay 1.5 second to display the text
  lcd.clear();                                                         //Clear the LCD
  set_gyro_angles = false;
  blevel = true;                                                                              
  bcalibrate = true; 
  count =0;                                        
 }                                                                  

void loop()
{
 if (bcalibrate)
   { docalibration();
    bcalibrate = false;
    //do not start measuring until calibration finished
    lcd.clear();                                                        
    lcd.setCursor(0,0);
    lcd.print("Correcting level");
    delay(2000); 

    tmpu =0;
    count= 0;
   }
 if ((tmpu >DTmpu) and !bcalibrate )
  { 
    //correct leveling mooi plat krijg je niet 0 je moet als hij level is offset bepalen
    read_mpu_6050_data();  //Read the raw acc and gyro data from the MPU-6050
    if (blevel) 
      { correctlevel();
        //Place the MPU-6050 spirit level and note the values in the following two lines for calibration                                            
       correctpitch  += angle_pitch_acc;
       correctroll  +=  angle_roll_acc;
       count +=1;
       if (count > 100)
         { correctpitch = correctpitch/100.0;
           correctroll = correctroll/100.0;
           //init voor visualisatie
          lcd.clear();                                                         //Clear the LCD
          lcd.setCursor(0,0);                                                  //Set the LCD cursor to position to position 0,0
          lcd.print("Pitch:");                                                 //Print text to screen
          lcd.setCursor(0,1);                                                  //Set the LCD cursor to position to position 0,1
          lcd.print("Roll :"); 
          blevel = false ;
          }
      }                                     
    else {calculate();} 
    //reset timer
    tmpu =0;
  }
 if ((tlcd > intervallcd) and !bcalibrate and !blevel)
   {
    //write_LCD();
    lcd.setCursor(6,0);
    lcd.print(angle_pitch_output);        
    lcd.setCursor(6,1);
    lcd.print(angle_roll_output);
    tlcd=0;     
   }
 if (tcycle > DTmpu ) 
    { Serial.print(">");
      Serial.print( DTmpu);
      Serial.println("ms");      
      Serial.println(tcycle);
    }  
   //debug
  // Serial.println( "====================");
  // Serial.print( "gyrogain:");
  // Serial.println( gyrogain,10);
  // Serial.print( "gyrogainRad:");
  // Serial.println(gyrogainRad,10);
  // Serial.println( "====================");   
   tcycle = 0;                                         
}
