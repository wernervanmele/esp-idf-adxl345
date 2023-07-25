# ADXL345 Accelerometer  
Still rough around the edges but the basics work :-)  
  
### Tested
- Framework:    ESP-IDF v5.1
- MCU:          Wemos ESP32-C3 Mini

### Implemented  
- X,Y,Z raw values  
- X,Y,Z values in m/s2
- Set Data Rate and Bandwidth rate
- Interrupts not implemented yet
- Only I2C Implemented, SPI seems a major PITA on the ESP framework.

### Get Started
```console
idf.py create-project <name-of-your-project>  
cd <name-of-your-project> 
idf.py set-target esp32     (depends on your board, esp32s2, esp32s3, esp32c3)  
mkdir components ; cd components  
git clone https://github.com/ropg/i2c_manager.git  
Read my I2C_MANAGER_README.md to apply fixes.  
git clone https://github.com/wernervanmele/esp-idf-adxl345.git esp-idf-adxl345
cd ..  
idf.py menuconfig  
- Component config > I2C Port Settings > do your I2C configuration  
- Component config > ADXL345 Digital Acccelerometer > 
- -  Enable the sensor module  
- - Set your I2C Address or keep the default.  
  
Save & Quit menuconfig.    
```



#### sources
- https://github.com/adafruit/Adafruit_ADXL345
- https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library/