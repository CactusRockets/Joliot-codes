#include "FastIMU.h"

/* CONFIGURAÇÕES MPU */
#define IMU_ADDRESS 0x68

#define PERFORM_CALIBRATION true

MPU6050 IMU;

calData calib = { 0 };
AccelData IMUAccel;
GyroData IMUGyro;

void calibrate() {
  println("Calibrando...");
  println("Keep IMU level.");
  delay(5000);
  if (IMU.hasMagnetometer()) {
    IMU.calibrateMag(&calib);
  }
  IMU.calibrateAccelGyro(&calib);
  println("Calibration done!");
  println("Accel biases X/Y/Z: ");
  print(calib.accelBias[0]);
  print(", ");
  print(calib.accelBias[1]);
  print(", ");
  println(calib.accelBias[2]);
  println("Gyro biases X/Y/Z: ");
  print(calib.gyroBias[0]);
  print(", ");
  print(calib.gyroBias[1]);
  print(", ");
  println(calib.gyroBias[2]);
  if (IMU.hasMagnetometer()) {
    println("Mag biases X/Y/Z: ");
    print(calib.magBias[0]);
    print(", ");
    print(calib.magBias[1]);
    print(", ");
    println(calib.magBias[2]);
    println("Mag Scale X/Y/Z: ");
    print(calib.magScale[0]);
    print(", ");
    print(calib.magScale[1]);
    print(", ");
    println(calib.magScale[2]);
  }
  IMU.init(calib, IMU_ADDRESS);
  println("Calibrado!");
}

void setupMPU() {
  int err = IMU.init(calib, IMU_ADDRESS);
  if (err != 0) {
    print("Error initializing IMU: ");
    println(err);
    while (true);
  }
  println("MPU6050 conectado!");

  if(PERFORM_CALIBRATION) {
    calibrate();
  }
}

void readMPU() {
  IMU.update();
  IMU.getAccel(&IMUAccel);
  IMU.getGyro(&IMUGyro);

  // Por enquanto aceleracao no sistema inercial do sensor
  allData.imuData.accelX = IMUAccel.accelX;
  allData.imuData.accelY = IMUAccel.accelY;
  allData.imuData.accelZ = IMUAccel.accelZ;
}