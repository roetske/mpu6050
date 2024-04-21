# mpu6050
working with mpu6050<br />
Measure acc x y z---- =>vibration measurement<br />
Measure angles turning =>balancing<br />
Gyro units = deg/s*s<br />
Acc units = g  9.81m/s*s<br />
See cherry file (*.ctd).

What a ride to get it working correct :).<br />
    - settings mpu6050 conf see cherry file.<br />
    - issues calibration <br />
        -accz average = (value -g) /samples <br />
    - formulas jeezes got lost in the beginning what is correct. <br />
            - if you want to measure vibrations you only need acc values - offset_acc <br />
               no formulas needed just have to know acc values are in g units.<br />
            - angles (roll,pitch,yaw) are combination of acc and gyro <br />
                   - and here you need your formulas <br />
                   - gyro values have to be measured at fix samplerate for calculations.<br />
            - Yaw is not stable with mpu6050 due to the fact no magnetometer.<br />   
            
Roll and pitch work fine now and stable.<br /> 
Provided button for calibration. <br /> 
Save offsets calibration in flash, so when restarted you do not need to recalibrate.
# hardware
mpu6050 i2c <br />
levelshifter <br />
lcd 20x4 with i2c <br />
button to select calibration <br />
led blink not stuck <br />

# Short explanation project

Display of roll and pitch of sensor mpu6050 on lcd. <br />
Pushbutton to calibrate. <br />
Special  in program each time you calibrate with pushbutton values are saved into flash. <br />
So when esp is restarted it reads the values in flash and you do not have to recalibrate. <br />
It also means that you do not have to recompile when calibration offsets change for whatever reason. <br />


#  Links
1)joop brokken natural wonder explaining how : https://www.youtube.com/watch?v=4BoIE8YQwM8 <br />
2)Adafuit mpu6050 one of the most extensive and well written libs but i struggled to measure angles.<br />
     Cycletime >50ms not good for me. <br />
3) MadgwickAHRS Filter Algorithm<br />
          http://x-io.co.uk/open-source-imu-and-ahrs-algorithms/<br />
4)Jeff Rowberg's MPU6050<br />
          https://github.com/jrowberg/i2cdevlib<br />
5)Kris Winer's MPU6050<br />
          https://github.com/kriswiner/MPU6050 <br /> 
6)Davide Gironi's AVR atmega MPU6050<br/>
          http://davidegironi.blogspot.com/2013/02/avr-atmega-mpu6050-gyroscope-and.html#.W7zM7mgzaUk  <br />
7) link youtube => https://www.youtube.com/watch?v=PjZWiBhST1U&t=35s  <br />

 # Notes :<br />
Cherrytree freeware program to make project file. Top tool to keep notes for your projects. *.ctd extension <br />
Tip use tab in arduino ide to write subs in separate tabs. makes coding it bit easier.<br />
Tip load library in sketch folder #include "somelib.h" it will load from sketchfolder instead of libraries in arduino <br />
    <somelib.h> will load from standard location "somelib.h" will load from sketchfolder <br />
Tip when using functions tan sin etc in arduino angles are in rad!!!.<br />

