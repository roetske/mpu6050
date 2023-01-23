void readFlashOffsetcal()
{ // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
  preferences.begin("data",false);
  GyX_cal = preferences.getFloat("gx", 99.0);;                                              
  GyY_cal = preferences.getFloat("gy", 99.0);                     
  GyZ_cal = preferences.getFloat("gz", 99.0);                                           
  AcX_cal = preferences.getFloat("ax", 99.0);
  AcY_cal = preferences.getFloat("ay", 99.0);
  AcZ_cal = preferences.getFloat("az", 99.0);
  preferences.end();
  if ( GyX_cal == 99.0 )
    {//need calibration
    Serial.println("Offsets from flash empty nok :(");
    bcalibrate = true;
    }
   else 
   { //is calibrated load floats with values from flash
     Serial.println("Offsets loaded from flash :)");
     MPU6050_AXOFFSET =  AcX_cal ;
     MPU6050_AYOFFSET =  AcY_cal ;
     MPU6050_AZOFFSET =  AcZ_cal;
     MPU6050_GXOFFSET  = GyX_cal;
     MPU6050_GYOFFSET  = GyY_cal; 
     MPU6050_GZOFFSET  = GyZ_cal;
     bcalibrate = false;
     //init for visualisation
    lcd.clear();                                                        
    lcd.setCursor(0,0);                                                 
    lcd.print("Pitch:");                                                 
    lcd.setCursor(0,1);                                                  
    lcd.print("Roll :");
     } 
}

void writeFlashOffsetcal()
{ // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.
  
  //debug excessive writing protect
  if (countwrite > maxwrite)
    { Serial.print("Writeprotect Countwriting >");
      Serial.println(countwrite);
      return;
    }
  countwrite +=1;      
  //wipe old data
  preferences.clear();
  preferences.begin("data",false);
  preferences.putFloat("gx", GyX_cal );;                                         
  preferences.putFloat("gy", GyY_cal);                                         
  preferences.putFloat("gz",GyZ_cal);                                           
  preferences.putFloat("ax",AcX_cal);
  preferences.putFloat("ay",AcY_cal);
  preferences.putFloat("az",AcZ_cal );
  preferences.end();
}

void docalibration()
{
  lcd.clear();                                                         //Clear the LCD
  lcd.setCursor(0,0);                                                  //Set the LCD cursor to position to position 0,0
  lcd.print("Calibrating gyro");                                       //Print text to screen
  lcd.setCursor(0,1);                                                  //Set the LCD cursor to position to position 0,1
  for (int cal_int = 0; cal_int < 2000 ; cal_int ++)                   //Run this code 2000 times
    {                  
      if(cal_int % 125 == 0)
        {lcd.print("."); }                                              //Print a dot on the LCD every 125 readings
      mpu6050_GetData();
      //debug
      //showvalues();                                                 //Read the raw acc and gyro data from the MPU-6050
      GyX_cal += GyX;                                              //Add the gyro x-axis offset to the GyX_cal variable
      GyY_cal += GyY;                                              //Add the gyro y-axis offset to the GyY_cal variable
      GyZ_cal += GyZ;                                              //Add the gyro z-axis offset to the GyZ_cal variable
      AcX_cal += AcX;
      AcY_cal += AcY;
      AcZ_cal += AcZ;
      delay(DTmpu);                                                 //Delay same as samplerate important !!!    
    }
                                                                     
  GyX_cal =  GyX_cal/2000.0;                                      
  GyY_cal  = GyY_cal/2000.0;                                      
  GyZ_cal =  GyZ_cal/2000.0;
  AcX_cal =   AcX_cal/2000.0;
  AcY_cal =   AcY_cal/2000.0;
  AcZ_cal =   (AcZ_cal/2000.0) - MPU6050_AccGAIN;  //1g 8g setting 4096 value =1g plat
  
  writeFlashOffsetcal();
  //load results in offset for calculation
  MPU6050_AXOFFSET =  AcX_cal ;
  MPU6050_AYOFFSET =  AcY_cal ;
  MPU6050_AZOFFSET = AcZ_cal;
  MPU6050_GXOFFSET  = GyX_cal;
  MPU6050_GYOFFSET  = GyY_cal; 
  MPU6050_GZOFFSET  = GyZ_cal; 
     
  lcd.clear();
   //lcd.setCursor(col, row)
  lcd.setCursor(0,0); 
  lcd.print("GyX_cal=");
  lcd.print(GyX_cal);
  lcd.setCursor(0,1); 
  lcd.print("GyY_cal=");
  lcd.print(GyY_cal);
  lcd.setCursor(0,2); 
  lcd.print("GyZ_cal=");
  lcd.print(GyZ_cal);
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("AcX_cal=");
  lcd.print(AcX_cal);
  lcd.setCursor(0,1); 
  lcd.print("AcY_cal=");
  lcd.print(AcY_cal);
  lcd.setCursor(0,2); 
  lcd.print("AcZ_cal=");
  lcd.print(AcZ_cal);  
  delay(5000); 
//init for visualisation
  lcd.clear();                                                        
  lcd.setCursor(0,0);                                                 
  lcd.print("Pitch:");                                                 
  lcd.setCursor(0,1);                                                  
  lcd.print("Roll :");                                     
}

void showvalues()
{ 
  Serial.print("Gyx:");
  Serial.print( GyX);
  Serial.print(",");
  Serial.print("GyY:");
  Serial.print( GyY);
  Serial.print(",");
  Serial.print("GyZ:");
  Serial.print( GyZ);
  Serial.println("");
  Serial.print("AcX:");
  Serial.print( AcX);
  Serial.print(",");
  Serial.print("AcY:");
  Serial.print( AcY);
  Serial.print(",");
  Serial.print("AcZ:");
  Serial.print( AcZ);
  Serial.println("");  
}

//this is complex above my paygrade lol i use above calibration code
// Function which accumulates gyro and accelerometer data after device initialization. It calculates the average
// of the at-rest readings and then loads the resulting offsets into accelerometer and gyro bias registers.
void calibrateMPU6050(float * dest1, float * dest2)
{  
  uint8_t data[12]; // data array to hold accelerometer and gyro x, y, z, data
  uint16_t ii, packet_count, fifo_count;
  int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
  
// reset device, reset all registers, clear gyro and accelerometer bias registers
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_1, 0x80); // Write a one to bit 7 reset bit; toggle reset device
  delay(100);  
   
// get stable time source
// Set clock source to be PLL with x-axis gyroscope reference, bits 2:0 = 001
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_1, 0x01);  
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_2, 0x00); 
  delay(200);
  
// Configure device for bias calculation
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_INT_ENABLE, 0x00);   // Disable all interrupts
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_FIFO_EN, 0x00);      // Disable FIFO
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_PWR_MGMT_1, 0x00);   // Turn on internal clock source
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_I2C_MST_CTRL, 0x00); // Disable I2C master
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_USER_CTRL, 0x0C);    // Reset FIFO and DMP
  delay(15);
  
// Configure MPU6050 gyro and accelerometer for bias calculation
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_CONFIG, 0x01);      // Set low-pass filter to 188 Hz
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity
 
  uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
  uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g

// Configure FIFO to capture accelerometer and gyro data for bias calculation
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_USER_CTRL, 0x40);   // Enable FIFO  
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO  (max size 1024 bytes in MPU-6050)
  delay(80); // accumulate 80 samples in 80 milliseconds = 960 bytes

// At end of sample accumulation, turn off FIFO sensor read
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
  readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_FIFO_COUNTH, 2, &data[0]); // read FIFO sample count
  fifo_count = ((uint16_t)data[0] << 8) | data[1];
  packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging

  for (ii = 0; ii < packet_count; ii++) {
    int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
    readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_FIFO_R_W, 12, &data[0]); // read data for averaging
    accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
    accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
    accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;    
    gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
    gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
    gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;
    
    accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
    accel_bias[1] += (int32_t) accel_temp[1];
    accel_bias[2] += (int32_t) accel_temp[2];
    gyro_bias[0]  += (int32_t) gyro_temp[0];
    gyro_bias[1]  += (int32_t) gyro_temp[1];
    gyro_bias[2]  += (int32_t) gyro_temp[2];
            
}
    accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
    accel_bias[1] /= (int32_t) packet_count;
    accel_bias[2] /= (int32_t) packet_count;
    gyro_bias[0]  /= (int32_t) packet_count;
    gyro_bias[1]  /= (int32_t) packet_count;
    gyro_bias[2]  /= (int32_t) packet_count;
    
  if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
  else {accel_bias[2] += (int32_t) accelsensitivity;}
 
// Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
  data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
  data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
  data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
  data[3] = (-gyro_bias[1]/4)       & 0xFF;
  data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
  data[5] = (-gyro_bias[2]/4)       & 0xFF;

// Push gyro biases to hardware registers
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_XG_OFFS_USRH, data[0]);// might not be supported in MPU6050
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_XG_OFFS_USRL, data[1]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_YG_OFFS_USRH, data[2]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_YG_OFFS_USRL, data[3]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ZG_OFFS_USRH, data[4]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ZG_OFFS_USRL, data[5]);

  dest1[0] = (float) gyro_bias[0]/(float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
  dest1[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
  dest1[2] = (float) gyro_bias[2]/(float) gyrosensitivity;

// Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
// factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
// non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
// compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
// the accelerometer biases calculated above must be divided by 8.

  int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
  readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_XA_OFFS_H, 2, &data[0]); // Read factory accelerometer trim values
  accel_bias_reg[0] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_YA_OFFS_H, 2, &data[0]);
  accel_bias_reg[1] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  readBytes(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ZA_OFFS_H, 2, &data[0]);
  accel_bias_reg[2] = (int16_t) ((int16_t)data[0] << 8) | data[1];
  
  uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
  uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis
  
  for(ii = 0; ii < 3; ii++) {
    if(accel_bias_reg[ii] & mask) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
  }

  // Construct total accelerometer bias, including calculated average accelerometer bias from above
  accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
  accel_bias_reg[1] -= (accel_bias[1]/8);
  accel_bias_reg[2] -= (accel_bias[2]/8);
 
  data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
  data[1] = (accel_bias_reg[0])      & 0xFF;
  data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
  data[3] = (accel_bias_reg[1])      & 0xFF;
  data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
  data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
  data[5] = (accel_bias_reg[2])      & 0xFF;
  data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

  // Push accelerometer biases to hardware registers
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_XA_OFFS_H, data[0]); // might not be supported in MPU6050
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_XA_OFFS_L_TC, data[1]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_YA_OFFS_H, data[2]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_YA_OFFS_L_TC, data[3]);  
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ZA_OFFS_H, data[4]);
  writeByte(MPU6050_DEFAULT_ADDRESS, MPU6050_RA_ZA_OFFS_L_TC, data[5]);

// Output scaled accelerometer biases for manual subtraction in the main program
   dest2[0] = (float)accel_bias[0]/(float)accelsensitivity; 
   dest2[1] = (float)accel_bias[1]/(float)accelsensitivity;
   dest2[2] = (float)accel_bias[2]/(float)accelsensitivity;
}