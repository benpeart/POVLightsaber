#include "globals.h"
#include "debug.h"
#include "MPU6050_DMP.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

static MPU6050_6Axis_MotionApps20 mpu;
static uint16_t packetSize;                // expected DMP packet size (default is 42 bytes)
static volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high

void dmpDataReady()
{
  mpuInterrupt = true;
}

uint8_t MPU6050_setup()
{
  // This has to happen _very_ early with the latest (6.5.0) platform
  // Gyro setup (utilize maximum I2C bus speed supported by the MPU6050 - 400kHz)
  Wire.begin();
  Wire.setClock(400000UL);

  mpu.initialize();
  if (!mpu.testConnection())
  {
    DB_PRINTLN("Failed to find MPU6050 chip!");
    return 1;
  }
  DB_PRINTLN("MPU6050 Found");
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);

  // configure the MPU6050 Digital Motion Processor (DMP)
  uint8_t devStatus; // return status after each device operation (0 = success, !0 = error)
  devStatus = mpu.dmpInitialize();
  if (devStatus == 0)
  {
    // turn on the DMP, now that it's ready
    DB_PRINTLN("Enabling DMP...");
    mpu.setDMPEnabled(true);

    // enable interrupt detection
    DB_PRINTF("Enabling interrupt detection (Arduino external interrupt %d)\n", PIN_MPU_INTERRUPT);
    attachInterrupt(PIN_MPU_INTERRUPT, dmpDataReady, RISING);
    DB_PRINTLN("DMP ready! Waiting for first interrupt...");

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
    DB_PRINTF("dmpGetFIFOPacketSize = %d\n", packetSize);
  }
  else
  {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    DB_PRINTF("DMP Initialization failed (code %d)\n", devStatus);
  }

  return devStatus;
}

void MPU6050_calibrate()
{
  DB_PRINTLN("PID tuning - each dot = 100 readings");
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
#ifdef DEBUG
  mpu.PrintActiveOffsets();
#endif
  DB_PRINTLN(" Calibration complete and stored in MPU6050");
}

float MPU6050_getPitch()
{
  uint16_t fifoCount;     // count of all bytes currently in FIFO
  uint8_t fifoBuffer[64]; // FIFO storage buffer
  float pitch;

  // orientation/motion vars
  Quaternion q;                    // [w, x, y, z]         quaternion container
  VectorFloat gravity;             // [x, y, z]            gravity vector
  static float ypr[3] = {0, 0, 0}; // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

  // wait until we have a complete packet, should be a VERY short wait
  fifoCount = mpu.getFIFOCount();
  while (fifoCount < packetSize)
    fifoCount = mpu.getFIFOCount();

  // Get the quaternion from the FIFO packet
  mpu.getFIFOBytes(fifoBuffer, packetSize);
  mpu.dmpGetQuaternion(&q, fifoBuffer); // get value for q

  // Compute gravity vector and YPR from the quaternion
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

  // Pitch in degrees
  pitch = ypr[1] * 180 / M_PI;

#ifdef DEBUG_IMU
  DB_PRINT(">yaw:");
  DB_PRINTLN(ypr[0] * 180 / M_PI);
  DB_PRINT(">pitch:");
  DB_PRINTLN(ypr[1] * 180 / M_PI);
  DB_PRINT(">roll:");
  DB_PRINTLN(ypr[2] * 180 / M_PI);
  DB_PRINT(">pitch:");
  DB_PRINTLN(pitch);
#endif // DEBUG_IMU

  return pitch;
}

bool MPU6050_newData()
{
  uint16_t fifoCount;   // count of all bytes currently in FIFO
  uint8_t mpuIntStatus; // holds actual interrupt status byte from MPU

  // If we have new motion processor data
  fifoCount = mpu.getFIFOCount();
  if (mpuInterrupt || fifoCount >= packetSize)
  {
    mpuInterrupt = false;

    // Check for FIFO overflow
    mpuIntStatus = mpu.getIntStatus();
    if ((mpuIntStatus & (1 << MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || fifoCount >= 1024)
    {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      DB_PRINTLN("FIFO overflow!");
    }
    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    else if (mpuIntStatus & (1 << MPU6050_INTERRUPT_DMP_INT_BIT))
    {
      return true;
    }
  }

  return false;
}
