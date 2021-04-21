/*
  SD card datalogger for High Stakes

  modified 2/24/21
  by Daniel Alfaro R.

*/

#include <SPI.h>
#include <SD.h>
#include "DHT.h"
#include <SFE_BMP180.h>
#include <Wire.h>
#include "RTClib.h"

#define DHTPIN 2        // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11

File myFile;
const int chipSelect = 10;
SFE_BMP180 pressure;
RTC_DS1307 rtc;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  #ifndef ESP8266
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  #endif

  dht.begin();
  
  if (!SD.begin(10) || !pressure.begin()) {
//    Serial.println("initialization failed!");
    while (1);
  }
  if (! rtc.begin()) {
    abort();
  }

  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
}

void loop() {
  char status;
  double T,P;
  // Wait a few seconds between measurements.
  delay(3000);

  DateTime now = rtc.now();
  String date = String(now.year()) + '/' + String(now.month()) + '/' + String(now.day()) + " " + String(now.hour()) + ':' + String(now.minute()) + ':' + String(now.second());

  //START BMP
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        status = pressure.getPressure(P,T);
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  //END BMP

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
//    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float hic = dht.computeHeatIndex(t, h, false);
  String completeData;
  completeData = String(date) + "," + String(t) + "," + String(h) + "," + String(hic) + "," + String(P);

  //Add header row
  if(!SD.exists("data.csv")){
    String headerRow = "Time, Temperature (Celsius), Humidity (% RH), Heat Index (Celsius), Pressure (mb)";
    myFile = SD.open("data.csv", FILE_WRITE);
  
    // if the file opened okay, write to it:
    if (myFile) {
//      Serial.print("Writing to data.csv...");
      myFile.println(headerRow);
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
//      Serial.println("error opening data.csv for header initialization.");
    }
  }
  myFile = SD.open("data.csv", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    myFile.println(completeData);
    // close the file:
    myFile.close();
  }
}