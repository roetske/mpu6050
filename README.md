# mpu6050
working with mpu6050

Working with gyros and accelerometer.
What can you do with accelerometer?
Measure acc x y z---- =>vibration measurement
Measure angles turning =>balancing

Mpu6050 or adxl345 are good starters.<br />
Coding so many examples and once you start you get stuck so easy at least i did.<br />
Setup gyro <br />
/131   +-250 °/s<br />
/65.5  +500)°/s<br />
/32.8  +1000°/s<br />
/16.4  +2000°/s<br />
Setup acc<br />
2g 4g 8 g 16g<br />
16384 2g<br />
8192 4g<br />
4096 8g<br />
2048 16g<br />
filters did not explore this yet<br />

gyro values deg/s<br />
acc in g<br />
you have extra value temperature you can read from mpu6050.<br />

Watch out you have to convert to rad when using tan sin etc in arduino angles are in rad!!!.<br />

# applications

Vibrations acc values. <br />
    acc rawdata /16384 settingfor setting 2g gives you g value (note when flat accz = 10 = g accx accy should be 0.<br />
When measuring angles you need to refresh at fix sample rate.<br />
   See code calculation in code. used calc from example joop.<br />
 Measure speed =>no you cannot measure speed with mpu6050.<br />

Use of filter to stabilize your readings.<br />

#  Links
1)joop brokken natural wonder explaining how : https://www.youtube.com/watch?v=4BoIE8YQwM8 <br />
2)Adafuit mpu6050 one of the most extensive and well written libs but i struggled to measure angles.<br />
     Cycletime >50ms not good for me. <br />
3)https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050 <br />
     what a lib but found it hard to use (must be me) problem cannot load lib you have to use include with "" <br />
Note:<br />
Cherrytree freeware program to make project file. Top tool to keep notes for your projects.<br />
Tip include "somelib.h" when this file is in your sketch folder it will load. Stumbled on this during project nice to know.<br />
Tip use tab in arduino ide to write subs in separate tabs. makes coding it bit easier.<br />


