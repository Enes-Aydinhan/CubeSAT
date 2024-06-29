#include <Adafruit_BMP085.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_SGP40.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_L3GD20_U.h>
#include <LSM303.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "DHT.h"
#define DHTPIN A2
#define DHTTYPE DHT11
#include <SD.h>


static const int RXPin = D0, TXPin = D1;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();  }


DHT dht(DHTPIN, DHTTYPE);

Adafruit_SGP40 sgp;


// SGP40 and DHT11 sensor values.
int sgp40_raw;
float humi;
float temp;

int voc_index;


// Functions to read from sensors and return values as strings.
String readSGPReading() {
    sgp40_raw = sgp.measureRaw();
    return String(sgp40_raw);
}

String readHumi() {
    humi = dht.readHumidity();
    return String(humi);
}

String readTemp() {
    temp = dht.readTemperature();
    return String(temp);
}

String readVOCIndex() {
    voc_index = sgp.measureVocIndex(temp, humi);
    return String(voc_index);
}

Adafruit_BMP085 bmp;

Adafruit_L3GD20_Unified gyro = Adafruit_L3GD20_Unified(20);

void displaySensorDetails(void)
{
  sensor_t sensor;
  gyro.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" rad/s");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" rad/s");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" rad/s");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

LSM303 compass;

char report[80];

//SD CARD VARIABLES
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = D6;
File myFile;

void setup() {
Wire.setSCL(D5);  
Wire.setSDA(D4);

SPI.setMISO(D10);
SPI.setMOSI(D2);
SPI.setSCLK(A1);

Serial.begin(9600);

 Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);

ss.begin(GPSBaud);


  if (!sgp.begin()){
        Serial.println("SGP40 sensor not found");
        while(100);
    }

    dht.begin();

  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }

  gyro.enableAutoRange(true);
  if(!gyro.begin())
  {
    /* There was a problem detecting the L3GD20 ... check your connections */
    Serial.println("Ooops, no L3GD20 detected ... Check your wiring!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();

  compass.init();
  compass.enableDefault();
}
  
void loop() {
  
  while (ss.available() > 0){
    if (gps.encode(ss.read())){
      Serial.println("");
      displayInfo();
      Serial.println("");
      }
  }
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }


    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    
    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

    Serial.print("Pressure at sealevel (calculated) = ");
    Serial.print(bmp.readSealevelPressure());
    Serial.println(" Pa");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");
    
    Serial.println();
    delay(500);

  Serial.print("DHT11 Temp: ");
  Serial.println(readTemp());
  Serial.print("DHT11 Humi: ");
  Serial.println(readHumi());
  Serial.println("");
    
  Serial.print("SGP READ: ");
  Serial.println(readSGPReading());
  Serial.print("voc: ");
  Serial.println(readVOCIndex());
  Serial.println("**********************************************");
  Serial.println("");


  /* Get a new sensor event */ 
  sensors_event_t event; 
  gyro.getEvent(&event);
 
  /* Display the results (speed is measured in rad/s) */
  Serial.print("X: "); Serial.print(event.gyro.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.gyro.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.gyro.z); Serial.print("  ");
  Serial.println("rad/s ");
  Serial.println("");

 compass.read();

  snprintf(report, sizeof(report), "A: %6d %6d %6d    M: %6d %6d %6d",
    compass.a.x, compass.a.y, compass.a.z,
    compass.m.x, compass.m.y, compass.m.z);
  Serial.println(report);
  Serial.println("");


myFile = SD.open("CUBESAT_DATALOG.txt", FILE_WRITE);
myFile.println();
myFile.println();
myFile.println("*********START_LOG*********");

myFile.println("***LSM303D OUT: ***");
myFile.println(report);

myFile.println("***L3GD20 OUT: ***");
myFile.print("X: "); myFile.print(event.gyro.x); myFile.print("  ");
myFile.print("Y: "); myFile.print(event.gyro.y); myFile.print("  ");
myFile.print("Z: "); myFile.print(event.gyro.z); myFile.print("  ");
myFile.println("rad/s ");
myFile.println("");

myFile.println("***SGP40 OUT: ***");
myFile.print("SGP RAW: ");
myFile.println(readSGPReading());
myFile.print("VOC INDEX: ");
myFile.println(readVOCIndex());
myFile.println("");

myFile.println("***DHT11 OUT: ***");
myFile.print("DHT11 Temp: ");
myFile.println(readTemp());
myFile.print("DHT11 Humi: ");
myFile.println(readHumi());
myFile.println("");

myFile.println("***BMP180 OUT: ***");
myFile.print("Temperature = ");
myFile.print(bmp.readTemperature());
myFile.println(" *C");
myFile.print("Pressure = ");
myFile.print(bmp.readPressure());
myFile.println(" Pa");    
myFile.print("Altitude = ");
myFile.print(bmp.readAltitude());
myFile.println(" meters");
myFile.print("Pressure at sealevel (calculated) = ");
myFile.print(bmp.readSealevelPressure());
myFile.println(" Pa");
myFile.print("Real altitude = ");
myFile.print(bmp.readAltitude(101500));
myFile.println(" meters");    
myFile.println();

myFile.println("***NEO-7M GPS OUT: ***");
myFile.print("LOCATION");
  if (gps.location.isValid())
  {
    myFile.print(gps.location.lat(), 6);
    myFile.print(",");
    myFile.print(gps.location.lng(), 6);
  }
  else
  {
myFile.print("***INVALID LOCATION***");}
myFile.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    myFile.print(gps.date.month());
    myFile.print(F("/"));
    myFile.print(gps.date.day());
    myFile.print(F("/"));
    myFile.print(gps.date.year());
  }
  else
  {
    myFile.print(F("INVALID DATE!"));
  }

  myFile.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) myFile.print(F("0"));
    myFile.print(gps.time.hour());
    myFile.print(F(":"));
    if (gps.time.minute() < 10) myFile.print(F("0"));
    myFile.print(gps.time.minute());
    myFile.print(F(":"));
    if (gps.time.second() < 10) myFile.print(F("0"));
    myFile.print(gps.time.second());
    myFile.print(F("."));
    if (gps.time.centisecond() < 10) myFile.print(F("0"));
    myFile.print(gps.time.centisecond());
  }
  else
  {
    myFile.print(F("INVALID TIME!"));
  }



myFile.close();
}
