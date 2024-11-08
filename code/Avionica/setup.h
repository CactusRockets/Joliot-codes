void setupComponents() {
  if(ENABLE_BUZZER) {
    setupBuzzer();
  }

  if(ENABLE_TELEMETRY) {
    setupTelemetry();
  }

  if(ENABLE_SD) {
    setupSd();
  }

  if(ENABLE_MPU) {
    setupMPU();
  }
  
  if(ENABLE_BMP) {
    setupBMP();
  }

  if(ENABLE_SKIBS) {
    setupSkibPins();
  }

  if(ENABLE_GPS) {
    setupGPS();
  }
}

void getSensorsMeasures() {
  // Medições BMP390
  if(ENABLE_BMP) {
    readBMP();
  }

  // Medições MPU6050
  if(ENABLE_MPU) {
    readMPU();
  }

  //Medições GPS
  if(ENABLE_GPS) {
    updateGPSData();
  }
}

void resetStructs() {
  allData = {
    { 0,0 },        // time, parachute
    { 0,0,0 },      // temperature, pressure, altitude
    { 0,0,0 },      // accX, accY, accZ
    { "", "", 0,0 }         // latitude, longitude
  };
  soloData = { 0 }; // openParachute
}