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
#define ENABLE_SD true
#define ENABLE_TELEMETRY true
#define ENABLE_GPS true

#define LED_ACTIVE 2

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

// import external files
#include "serial.h"
#include "buzzer.h"
#include "telemetry.h"
#include "moduleSD.h"
#include "bmp.h"
#include "imu.h"
#include "gps.h"
#include "parachute.h"
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

// Variáveis para controle de tempo
unsigned long lastTelemetryTime = 0;
// intervalo de transmissão do lora
const unsigned long telemetryInterval = 1200; 

void setup()
{
  Wire.begin();
  Wire.setClock(400000);

  sd_message.reserve(1500);
  telemetry_message.reserve(1500);

  Serial.begin(115200);

  setupComponents();
  getInitialAltitude();
  resetStructs();
  tripleBuzzerBip();

  pinMode(LED_ACTIVE, OUTPUT);
  digitalWrite(LED_ACTIVE, HIGH);

  delay(1000);
}

void loop()
{
  int executionTime = millis() / 1000;

  getSensorsMeasures();
  // Serial.println("IsDropping: " + String(isDropping));

  allData.data.time = millis() / 1000.0;

  checkApogee();
  saveMessages();

  println(telemetry_message);
  // debugTelemetryMessage(telemetry_message);

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
    unsigned long currentMillis = millis();
    if (currentMillis - lastTelemetryTime >= telemetryInterval)
    {
      lastTelemetryTime = currentMillis;

      transmit();
      if (hasSoloMessage())
      {
        receive();
      }
    }
  }
  delay(200);
}
