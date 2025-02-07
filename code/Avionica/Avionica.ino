// Projeto Joliot - Avionica 1Km

// import support libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>

#define ENABLE_SERIAL true
#define ENABLE_BUZZER true
#define ENABLE_BMP true
#define ENABLE_MPU true
#define ENABLE_SKIBS true
#define ENABLE_SD false
#define ENABLE_TELEMETRY false
#define ENABLE_GPS true

struct AvionicData
{
  float time;
  int parachute;
};

struct BmpData
{
  float temperature;
  float pressure;
  float altitude;
};

struct ImuData
{
  float accelX;
  float accelY;
  float accelZ;
  float quaternion_w;
  float quaternion_x;
  float quaternion_y;
  float quaternion_z;
};

struct GpsData
{
  String date;
  String hour;
  double latitude, longitude;
};

struct PacketData
{
  AvionicData data;
  BmpData bmpData;
  ImuData imuData;
  GpsData gpsData;
  int parachute;
};

struct SoloData
{
  int openParachute;
};

PacketData allData;
SoloData soloData;

String telemetry_message = "";
String sd_message = "";
String solo_message = "";

bool isBeeping = false;

bool setupSDFlag = false;
bool setupMPUFlag = false;
bool setupBMPFlag = false;
bool setupGPSFlag = false;

int package_counter = 0;

float initial_altitude;

// import external filess
#include "serial.h"    // debug prints
#include "buzzer.h"    // sinal sonoro
#include "telemetry.h" // telemetria
#include "moduleSD.h"  // armazenamento SD
#include "bmp.h"       // barometro
#include "imu.h"       // acelerometro
#include "gps.h"       // localizacao gps
#include "parachute.h" // ativacao paraquedas
#include "setup.h"
#include "debug.h"
#include "messages.h"

void setupComponents();
void getSensorsMeasures();
void beepIntermitating();
void activateParachutes();
void resetStructs();
void checkApogee();
void saveMessages();

void setup()
{
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

  // Inicializa as variáveis
  getInitialAltitude();
  resetStructs();
  tripleBuzzerBip();

  delay(1000);
}

void loop()
{
  getSensorsMeasures();
  // Serial.println("IsDropping: " + String(isDropping));

  // Armazena o tempo de execução
  allData.data.time = millis() / 1000.0;

  checkApogee();
  saveMessages();

  println(telemetry_message);
  debugTelemetryMessage(telemetry_message);

  if (ENABLE_SD)
  {
    verifySD();
    if (setupSDFlag) {
      writeOnSD(sd_message);
    } else {
      wrapperSetupSD();
    }
  }

  if (ENABLE_TELEMETRY)
  {
    transmit();
    if (hasSoloMessage())
    {
      receive();
    }
  }
}