void read_mpu_6050_data()
{                                                                       //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                                       //Wait until all the bytes are received
    acc_x = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_x variable
    acc_y = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_y variable
    acc_z = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_z variable
    temperature = Wire.read()<<8|Wire.read();                            //Add the low and high byte to the temperature variable
    gyro_x = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_x variable
    gyro_y = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_y variable
    gyro_z = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_z variable

}

void setup_mpu_6050_registers()
{
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0x10);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
}

void calculate()
{
    gyro_x -= gyro_x_cal;                                                //Subtract the offset calibration value from the raw gyro_x value
    gyro_y -= gyro_y_cal;                                                //Subtract the offset calibration value from the raw gyro_y value
    gyro_z -= gyro_z_cal;                                                //Subtract the offset calibration value from the raw gyro_z value
    
	// //acc correction                                                  //not needed
    // acc_x -= acc_x_cal;
    // acc_y -= acc_y_cal;
    // acc_z -= acc_z_cal;
    
    //Gyro angle calculations
    // voorbeeld 250Hz 0.0000611 = 1 / (250Hz) / 65.5)
    angle_pitch += gyro_x * gyrogain;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
    angle_roll += gyro_y * gyrogain;                                     //Calculate the traveled roll angle and add this to the angle_roll variable
    
    //voorbeeld /0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians 500 DEG SETTING GYRO
    angle_pitch += angle_roll * sin(gyro_z * gyrogainRad);               //If the IMU has yawed transfer the roll angle to the pitch angel
    angle_roll -= angle_pitch * sin(gyro_z * gyrogainRad);               //If the IMU has yawed transfer the pitch angle to the roll angel
    
    //Accelerometer angle calculations
    acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));  //Calculate the total accelerometer vector
    
    
    //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
    angle_pitch_acc = asin((float)acc_y/acc_total_vector)* RAD_TO_DEG;         //Calculate the pitch angle
    angle_roll_acc = asin((float)acc_x/acc_total_vector)* (-RAD_TO_DEG);       //Calculate the roll angle
    
    //Place the MPU-6050 spirit level and note the values in the following two lines for calibration
    angle_pitch_acc -=correctpitch;                                              //Accelerometer calibration value for pitch
    angle_roll_acc -= correctroll;                                               //Accelerometer calibration value for roll
  
    if(set_gyro_angles)
     {                                                                   //If the IMU is already started
      angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;     //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
      angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;        //Correct the drift of the gyro roll angle with the accelerometer roll angle
     }
    else
     {                                                                    //At first start
      angle_pitch = angle_pitch_acc;                                     //Set the gyro pitch angle equal to the accelerometer pitch angle 
      angle_roll = angle_roll_acc;                                       //Set the gyro roll angle equal to the accelerometer roll angle 
      set_gyro_angles = true;                                            //Set the IMU started flag
     }
    
    //To dampen the pitch and roll angles a complementary filter is used
    float damping =0.9;
    angle_pitch_output = angle_pitch_output * damping + angle_pitch * (1-damping);   //Take 90% of the output pitch value and add 10% of the raw pitch value
    angle_roll_output = angle_roll_output * damping + angle_roll * (1-damping);      //Take 90% of the output roll value and add 10% of the raw roll value
    
    //safety moet binnen grenzen blijven
    if (abs(angle_pitch_output) > 180)
      {angle_pitch_output= 0;}
    if (abs(angle_roll_output) > 180)
      {angle_roll_output= 0;}    
}




void docalibration()
{
  lcd.clear();                                                         //Clear the LCD
  lcd.setCursor(0,0);                                                  //Set the LCD cursor to position to position 0,0
  lcd.print("Calibrating gyro");                                       //Print text to screen
  lcd.setCursor(0,1);                                                  //Set the LCD cursor to position to position 0,1
  for (int cal_int = 0; cal_int < 2000 ; cal_int ++)                  //Run this code 2000 times
    {                  
      if(cal_int % 125 == 0)
        {lcd.print("."); }                                              //Print a dot on the LCD every 125 readings
      read_mpu_6050_data();
      
      showvaluesgyro();                                                 //Read the raw acc and gyro data from the MPU-6050
      gyro_x_cal += gyro_x;                                              //Add the gyro x-axis offset to the gyro_x_cal variable
      gyro_y_cal += gyro_y;                                              //Add the gyro y-axis offset to the gyro_y_cal variable
      gyro_z_cal += gyro_z;                                              //Add the gyro z-axis offset to the gyro_z_cal variable
      acc_x_cal += acc_x;
      acc_y_cal += acc_y;
      acc_z_cal += acc_z;
      delay(4);                                                          //Delay 3ms to simulate the 250Hz program loop
    }
                                                                       //Divide the gyro_z_cal variable by 2000 to get the avarage offset
  gyro_x_cal =  gyro_x_cal/2000.0;                                       //Divide the gyro_x_cal variable by 2000 to get the avarage offset
  gyro_y_cal  = gyro_y_cal/2000.0;                                       //Divide the gyro_y_cal variable by 2000 to get the avarage offset
  gyro_z_cal =  gyro_z_cal/2000.0;
  acc_x_cal =   acc_x_cal/2000.0;
  acc_y_cal =   acc_y_cal/2000.0;
  acc_z_cal =   acc_z_cal/2000.0;  
  lcd.clear();
   //lcd.setCursor(col, row)
  lcd.setCursor(0,0); 
  lcd.print("gyro_x_cal=");
  lcd.print(gyro_x_cal);
  lcd.setCursor(0,1); 
  lcd.print("gyro_y_cal=");
  lcd.print(gyro_y_cal);
  lcd.setCursor(0,2); 
  lcd.print("gyro_z_cal=");
  lcd.print(gyro_z_cal);
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("acc_x_cal=");
  lcd.print(acc_x_cal);
  lcd.setCursor(0,1); 
  lcd.print("acc_y_cal=");
  lcd.print(acc_y_cal);
  lcd.setCursor(0,2); 
  lcd.print("acc_z_cal=");
  lcd.print(acc_z_cal);  
  delay(5000);                                      
}


void correctlevel()
{   gyro_x -= gyro_x_cal;                                                //Subtract the offset calibration value from the raw gyro_x value
    gyro_y -= gyro_y_cal;                                                //Subtract the offset calibration value from the raw gyro_y value
    gyro_z -= gyro_z_cal;                                                //Subtract the offset calibration value from the raw gyro_z value
    
	//  //acc correction                                                 //nok better with level function
    // acc_x -= acc_x_cal;
    // acc_y -= acc_y_cal;
    // acc_z -= acc_z_cal;
    //Gyro angle calculations
    // voorbeeld 250Hz 0.0000611 = 1 / (250Hz) / 65.5)
    angle_pitch += gyro_x * gyrogain;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
    angle_roll += gyro_y * gyrogain;                                     //Calculate the traveled roll angle and add this to the angle_roll variable
    
    //voorbeeld /0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians 500 DEG SETTING GYRO
    angle_pitch += angle_roll * sin(gyro_z * gyrogainRad);               //If the IMU has yawed transfer the roll angle to the pitch angel
    angle_roll -= angle_pitch * sin(gyro_z * gyrogainRad);               //If the IMU has yawed transfer the pitch angle to the roll angel
    
    //Accelerometer angle calculations
    acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));  //Calculate the total accelerometer vector
    
    
    //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
    angle_pitch_acc = asin((float)acc_y/acc_total_vector)* RAD_TO_DEG;         //Calculate the pitch angle
    angle_roll_acc = asin((float)acc_x/acc_total_vector)* (-RAD_TO_DEG);       //Calculate the roll angle
                                             
}

void showvaluesgyro()
{ 
  Serial.print("GyroX:");
  Serial.print( gyro_x);
  Serial.print(",");
  Serial.print("GyroY:");
  Serial.print( gyro_y);
  Serial.print(",");
  Serial.print("GyroZ:");
  Serial.print( gyro_z);
  Serial.println("");
}