// Avionica 1Km

// import support libraries
#include <SPI.h>
#include <Wire.h>

#define ENABLE_SERIAL true
#define ENABLE_BUZZER false
#define ENABLE_BMP true
#define ENABLE_MPU true
#define ENABLE_SKIBS false
#define ENABLE_SD false
#define ENABLE_TELEMETRY true
#define ENABLE_GPS true

#define ALAVANCA 26
#define ALAVANCA_BEEP_TIME 300

struct AvionicData {
  float time;
  int parachute;
};

struct BmpData {
  float temperature;
  float pressure;
  float altitude;
};

struct ImuData {
  float accelX;
  float accelY;
  float accelZ;
};

struct GpsData {
  double latitude, longitude;
};

struct PacketData {
  AvionicData data;
  BmpData bmpData;
  ImuData imuData;
  GpsData gpsData;
};

struct SoloData {
  int openParachute;
};

PacketData allData;
SoloData soloData;

String telemetry_message = "";
String sd_message = "";
String solo_message = "";

double alavancaTime = 0;
bool isBeeping = false;
bool alavancaAcionada = false;

int package_counter = 0;

float initial_altitude;

// import external files
#include "serial.h"     // debug prints
#include "buzzer.h"     // sinal sonoro
#include "telemetry.h"  // telemetria
#include "moduleSD.h"   // armazenamento SD
#include "bmp.h"        // barometro
#include "imu.h"        // acelerometro
#include "gps.h"        // localizacao gps
#include "parachute.h"  // ativacao paraquedas

void setupComponents();
void getSensorsMeasures();
void beepIntermitating();
void activateParachutes();
void resetStructs();
void checkApogee();
void saveMessages();

void setup() {
  // Inicializa biblioteca I2C
  Wire.begin();
  Wire.setClock(400000);

  // Reserva espaço de memoria para as mensagens, aumentando a perfomance
  sd_message.reserve(1500);
  telemetry_message.reserve(1500);

  // Inicializa a serial
  Serial.begin(115200);

  // Inicializa sensores e configura pinos
  setupComponents();

  // Zera todos os valores
  resetStructs();

  delay(1000);
}

void loop() {
  // if(digitalRead(ALAVANCA) == HIGH) {
  if(true) {
    if(alavancaAcionada == false) {
      initial_altitude = getAltitude();
      alavancaAcionada = true;
    }

    getSensorsMeasures();

    beepIntermitating();

    // Armazena o tempo de execução
    allData.data.time = millis() / 1000.0;

    checkApogee();
    saveMessages();

    println(telemetry_message);

    if(ENABLE_SD) {
      writeOnSD(sd_message);
    }

    if(ENABLE_TELEMETRY) {
      transmit();
      if(hasSoloMessage()) {
        receive();
      }
    }

    delay(500);
  }
}

void setupComponents() {
  pinMode(ALAVANCA, INPUT);

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

void beepIntermitating() {
  if(millis() - alavancaTime >= ALAVANCA_BEEP_TIME) {
    alavancaTime = millis();
    isBeeping = !isBeeping;
    
    if(isBeeping) {
      activateBuzzer();
    } else {
      desactivateBuzzer();
    }
  }
}

void resetStructs() {
  allData = {
    { 0,0 },        // time, parachute
    { 0,0,0 },      // temperature, pressure, altitude
    { 0,0,0 },      // accX, accY, accZ
    { 0,0 }         // latitude, longitude
  };
  soloData = { 0 }; // openParachute
}

String fixNumberSize(int num, int width, bool enableSignal=false){
  int numPositive = (num >= 0 ? num : -num);
  String formattedString = String(numPositive);

  while (formattedString.length() < width){
    formattedString = "0" + formattedString;
  }

  if(!enableSignal) return formattedString;

  if(num < 0) {
    formattedString = "-" + formattedString;
  } else {
    formattedString = "+" + formattedString;
  }

  return formattedString;
}

void saveMessages() {
  telemetry_message = telemetryMessage();
  sd_message = sdMessage();
  package_counter++;
}

String telemetryMessage(){
  // Padráo da Mensagem:
  // S -> Sinal (pode ser "+" ou "-")
  // P -> Numeracao do Pacote
  // A -> Altitude medida
  // Z -> Acelecerao no eixo Z
  // P -> Estado do Paraquedas
  // T -> Latitude do GPS
  // G -> Longitude do GPS
  // Ex.: PPPPPSAAAAAASZZZZPSTTTTTSGGGGG
  
  String packetString = fixNumberSize(package_counter, 5)
    + fixNumberSize((int) (allData.bmpData.altitude*1000), 6, true)
    + fixNumberSize((int) (allData.imuData.accelZ*100), 4, true)
    + String(allData.data.parachute);
    
  String gpsString = fixNumberSize((int) (allData.gpsData.latitude*1000), 5, true)
    + fixNumberSize((int) (allData.gpsData.longitude*1000), 5, true);

  return (packetString + gpsString);
}

String sdMessage(){
  String packetString = String(allData.data.time, 3)
    + "," + String(allData.bmpData.temperature, 2)
    + "," + String(allData.bmpData.pressure, 2)
    + "," + String(allData.bmpData.altitude, 3)
    + "," + String(allData.imuData.accelZ)
    + "," + String(allData.data.parachute);
    
  String gpsString = String(allData.gpsData.latitude, 3)
    + "," + String(allData.gpsData.longitude, 3);

  return (packetString + ',' + gpsString);
}