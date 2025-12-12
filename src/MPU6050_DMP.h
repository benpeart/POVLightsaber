#ifndef MPU6050_H
#define MPU6050_H

// 0 = success
// 1 = initial memory load failed
// 2 = DMP configuration updates failed
// (if it's going to break, usually the code will be 1)
uint8_t MPU6050_setup();
void MPU6050_calibrate();
bool MPU6050_newData();
float MPU6050_getPitch();

#endif // MPU6050_H
