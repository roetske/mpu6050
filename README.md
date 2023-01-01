# mpu6050
working with mpu6050

Working with gyros and accelerometer.
What can you do with accelerometer?
Measure acc x y z---- =>vibration measurement
Measure angles turning =>balancing

Mpu6050 or adxl345 are good starters.
Coding so many examples and once you start you get stuck so easy at least i did.
Setup gyro 
/131   +-250 °/s
/65.5  +500)°/s
/32.8  +1000°/s
/16.4  +2000°/s
Setup acc
2g 4g 8 g 16g
16384 2g
8192 4g
4096 8g
2048 169
filters did not explore this yet

gyro values deg/s
acc in g
you have extra value temperature you can read from mpu6050.

Watch out you have to convert to rad when using tan sin etc in arduino angles are in rad!!!.

# applications

Vibrations acc values. 
    acc rawdata /16384 settingfor setting 2g gives you g value (note when flat accz = 10 = g accx accy should be 0.
When measuring angles you need to refresh at fix sample rate.
   See code calculation in code. used calc from example joop.
 Measure speed =>no you cannot measure speed with mpu6050.

Use of filter to stabilize your readings.

#  Links
1)joop brokken natural wonder explaining how : https://www.youtube.com/watch?v=4BoIE8YQwM8 <br />
2)Adafuit mpu6050 one of the most extensive and well written libs but i struggled to measure angles.<br />
Cycletime >50ms not good for me. <br />
Note:<br />
Cherrytree freeware program to make project file. Top tool to keep notes for your projects.<br />
Tip include "somelib.h" when this file is in your sketch folder it will load. Stumbled on this during project nice to know.<br />
Tip use tab in arduino ide to write subs in separate tabs. makes coding it bit easier.<br />


