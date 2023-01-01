String getGyroReadings()
{
  mpu.getEvent(&a, &g, &temp);
  String gyroString;
  float gyroX_temp = g.gyro.x;
  if(abs(gyroX_temp) > gyroXerror)  {
    gyroX += gyroX_temp/50.00; //waarom die delingen average 50?
  }
  
  float gyroY_temp = g.gyro.y;
  if(abs(gyroY_temp) > gyroYerror) {
    gyroY += gyroY_temp/70.00;
  }

  float gyroZ_temp = g.gyro.z;
  if(abs(gyroZ_temp) > gyroZerror) {
    gyroZ += gyroZ_temp/90.00;
  }
  Serial.println(gyroX);
  Serial.println(gyroY);
  Serial.println(gyroZ);
    
  doc["gyroX"] = String(gyroX);
  doc["gyroY"] = String(gyroY);
  doc["gyroZ"] = String(gyroZ);

  serializeJson(doc,gyroString);
  Serial.print("gyroString=");
  Serial.println(gyroString);
  doc.clear();
  return gyroString;
}


String getAccReadings() 
{
  String accString;
  mpu.getEvent(&a, &g, &temp);
  // Get current acceleration values
  accX = a.acceleration.x;
  accY = a.acceleration.y;
  accZ = a.acceleration.z;
  Serial.println(accX);
  Serial.println(accY);
  Serial.println(accZ);  
  //  doc2["accX"].set(String(accX));
  //  doc2["accY"].set(String(accY));
  //  doc2["accZ"].set(String(accZ));
 
  doc2["accX"] = String(accX);
  doc2["accY"] = String(accY);
  doc2["accZ"] = String(accZ);
  
  serializeJson(doc2,accString);
  Serial.print("accString=");
  Serial.println(accString);
  doc2.clear();
  return accString;
}



String getTemperature(){
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
  return String(temperature);
}


//float to string geeft Null value
String floattostring(float floatValue)
{
  char buffer[20];
  dtostrf(floatValue, 0, 2, buffer);
  String floatString = String(buffer);
  return floatString;
}
