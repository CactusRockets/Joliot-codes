#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

/* CONFIGURAÇÕES GPS */
#define GPS_WAY 1

// Pinos da Serial 1 do ESP32
#define RX1_PIN 16
#define TX1_PIN 17

// Usando a Serial 1 do ESP32
HardwareSerial GPSSerial(2);
TinyGPSPlus gps;

String gpsData;

double latitude = 0, longitude = 0;
int ano, mes, dia, hora, minuto, segundo, centesimo;

void verifyGPS() {
  GPSSerial.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
  setupGPSFlag = !!GPSSerial;
}

void setupGPS() {
  GPSSerial.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);
  println("GPS conectado!");
}

void getLatitudeAndLongitude() {

  println("Sinais de GPS disponíveis: ");
  println(GPSSerial.available());

  if(GPSSerial.available()) {
    // Lê os dados recebidos do módulo GPS
    gpsData = GPSSerial.readStringUntil('\n');
    Serial.println(gpsData);
    
    // Verifica se os dados começam com "$GPGGA", que é 
    // uma sentença NMEA que contém as informações 
    // de latitude e longitude
    if(gpsData.startsWith("$GPGGA")) {

      // Divide os dados em campos usando a vírgula como separador
      // Os campos estão na seguinte ordem: 
      // $GPGGA, horário, latitude, N/S, longitude, E/W, qualidade do sinal GPS, número de satélites
      // No nosso caso, queremos o campo de latitude (índice 2) e o campo de longitude (índice 4)
      String fields[15];
      int fieldIndex = 0;
      int startPos = 0;
      int commaPos;
      
      // Loop para dividir os campos separados por vírgula
      while((commaPos = gpsData.indexOf(',', startPos)) != -1) {
        fields[fieldIndex] = gpsData.substring(startPos, commaPos);
        fieldIndex++;
        startPos = commaPos + 1;
      }
      
      // Extrai os valores de latitude e longitude e converte-os em valores numéricos
      latitude = fields[2].toFloat();
      longitude = fields[4].toFloat();
    }
  }
}

void getGPSData() {
  bool isNewGPSData = false;

  println("Sinais de GPS disponíveis: ");
  println(GPSSerial.available());

  while(GPSSerial.available() > 0) {
    char GPSData = GPSSerial.read();
    isNewGPSData = gps.encode(GPSData);
    println("isNewGPSData: ");
    Serial.println(isNewGPSData);
  }

  if(isNewGPSData) {
    if(gps.location.isValid()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }

    if(gps.date.isValid()) {
      mes = gps.date.month();
      dia = gps.date.day();
      ano = gps.date.year();
    }
    
    if(gps.time.isValid()) {
      hora = gps.time.hour();
      minuto = gps.time.minute();
      segundo = gps.time.second();
      centesimo = gps.time.centisecond();
    }
  }
}

void saveGPSData() {
  String date = String(dia) + "/" + String(mes) + "/" + String(ano);
  String hour = String(hora) + ":" + String(minuto) + ":" + String(segundo); 

  allData.gpsData.hour = hour;
  allData.gpsData.date = date;
  allData.gpsData.latitude = latitude;
  allData.gpsData.longitude = longitude;
}

void printData() {
  println(gpsData);
  
  print("Latitude: ");
  println(latitude, 6);
  print("Longitude: ");
  println(longitude, 6);

  print("Data: ");
  print(dia);
  print("/");
  print(mes);
  print("/");
  println(ano);
}

void updateGPSData() {
  if(GPS_WAY == 1) {
    getGPSData();
    getLatitudeAndLongitude();
  } else if(GPS_WAY == 2) {
    getLatitudeAndLongitude();
  }
  saveGPSData();
  printData();
}
