/*
 version 0.0.1 - 23 05 2020
 version 0.0.2 - 24 05 2020
 version 0.0.3 - 22 05 2020
 */
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SDM.h>
#include <Http.h>
#include <Sim800.h>
#include <Dusk2Dawn.h>

// Sensor
int sensorPin = A0;
float sensorValue = 0.0;
int YDHC = 1; //30A sensor
//int YDHC = 2;//15A sensor
//int YDHC = 6; //5A sensor
float corf = 0.026; // correction factor
float calf = 1.0; // callibration factor
float fcur = 0.0; // final result
char scur[6]; // string respresentation of result

// SIM800
#define POSTENDPOINT "http://gbstraathof.pythonanywhere.com/arduino/api/current/?format=json&username=testuser&api_key=testkey"
#define BODY_FORMAT "{\"current\": %s}"
char *dtostrf(double val, signed char width, unsigned char prec, char *s);

const int simPin =  45;
unsigned int RX_PIN = 52;
unsigned int TX_PIN = 53;
unsigned int RST_PIN = 12;
const char BEARER[] PROGMEM = "internet";
HTTP http(9600, RX_PIN, TX_PIN, RST_PIN);

// RTC & SD
RTC_DS1307 rtc;
File logFile;
char fout[] = "yyyymmdd.log"; // filename max 8 char, ext max 3

int tsec, tsecprev, smod, tmin, tminprev, mmod;
char datetime[] = "yyyy-mm-dd hh:mm:ss";
char logstring[28];

// Sampling intervals
int logint = 30; // logging interval (seconds)
int postint = 2; // posting interval (minutes)

// Dusk2Dawn
Dusk2Dawn ulv(51.542372, 4.829748, 1);
bool DST;

/*
 * setup
 */

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  Serial.println("initialization starting...");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  resetRTC();

  // CUSTOM PIN SELECT FOR THE MEGA = feature of SDM library
  if (!SD.begin(10, 11, 12, 13)) {
    Serial.println("initialization failed!");
    while (1);
  }

  DateTime now = rtc.now();
  setFilename(now);

  Serial.print("writing to ");
  Serial.println(fout);

  // Write line to indicate new measuring sequence
  logFile = SD.open(fout, FILE_WRITE);
  logFile.println("9999");
  logFile.close();

  // set DST flag
  DST = isDST(now);

  Serial.println("initialization done.");
}


/*
 * loop
 */

void loop() {

  DateTime now = rtc.now();

  tsec = now.second();

  while ( tsec != tsecprev) {

    tsecprev = tsec;
    smod = tsec % logint ;

    if ( smod == 0 ){

      readCurrent();
      setDTstring(now);

      Serial.print(datetime);
      Serial.print(" - Current (A): ");
      Serial.println(fcur);

      writeOut(now);

    }
  }

  // Every 'postint' minutes a sample will be posted
  tmin = now.minute();
  while ( tmin != tminprev) {

    tminprev = tmin;
    mmod = tmin % postint ;

    if ( mmod == 0 && isDaytime(now, ulv, DST, 30)){
      Serial.println("a sample will be sent to the server... ");

      postReading();

    }
  }

  // Wait 0.5 s before testing for the next 30 second interval
  delay(500);
}
