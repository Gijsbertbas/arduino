/* 15 dec 17 DW 2337h VER 0.1.0
 created dec 2017 en aangepast jan 2018 by dick w
 adapted feb 2018 by gijs 
 rewrite jan 2019 by gijs:
  - reset clock at setup
  - combine temp and hum in one logfile
  - new logfile every day
0.5.0:
  - post samples to server using HTTP library
 */
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <Http.h>
#include <Sim800.h>

// DHT
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

// SIM800
#define POSTENDPOINT "http://gbstraathof.pythonanywhere.com/arduino/api/dht/?format=json&username=***&api_key=***"
#define GETENDPOINT "http://gbstraathof.pythonanywhere.com/arduino/hw/"
#define BODY_FORMAT "{\"temp\": %s, \"hum\": %s}"
char *dtostrf(double val, signed char width, unsigned char prec, char *s);

//unsigned int RX_PIN = 7;
//unsigned int TX_PIN = 8;
//unsigned int RST_PIN = 12;
//HTTP http(9600, RX_PIN, TX_PIN, RST_PIN); 
HTTP http(9600, 7, 8, 12); 

RTC_DS1307 rtc;
File logFile;

float fhum, ftem;
int tsec, tsecprev, tmin, tminprev, tmod;
char fout[] = "yyyymmdd.log";
char datetime[] = "yyyy-mm-dd hh:mm:ss";

/*
 * functions
 */

void resetRTC() {
  DateTime dt = DateTime(2008, 1, 2, 1, 1, 1); // reset manually
  DateTime dt2 = DateTime(F(__DATE__),F(__TIME__)); // reset to computer time
  rtc.adjust(dt2);
}

void readDHT() {
  fhum = dht.readHumidity();
  ftem = dht.readTemperature();
}

void setFilename(DateTime now) {
  sprintf(fout, "%02d%02d%02d.log", now.year(), now.month(), now.day());
  if (SD.exists(fout)) {
    Serial.print(fout);
    Serial.println(" already exists.");
  } else{
    logFile = SD.open(fout, FILE_WRITE);
    logFile.println("YYYY-MM-DD HH:MM:SS Temperature(C) Humidity(%)");
    logFile.close();
    Serial.print(fout);
    Serial.println(" created.");
  }
}

void setDTstring(DateTime now) {
  sprintf(datetime, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}

void writeOut(DateTime now) {

  setFilename(now);
  
  logFile = SD.open(fout, FILE_WRITE);
  logFile.print(datetime);
  logFile.print(" ");
  logFile.print(ftem);
  logFile.print(" ");
  logFile.println(fhum);
  logFile.close();
}

void postReading() {
  char stem[5];
  char shum[5];
  dtostrf(ftem, 4, 2, stem);
  dtostrf(fhum, 4, 2, shum);

  char body[29];
  sprintf(body, BODY_FORMAT, stem, shum);
  Serial.println(body);

  http.configureBearer("internet"); 
  http.connect();

  // to allow the connection to fully establish, may not be needed:
  delay(2000); 
 
  char response[256];
  Result result;
  //result = http.get(GETENDPOINT, response);
  result = http.post(POSTENDPOINT, body, response);

  if (result == SUCCESS) {
    Serial.print("Post success!: ");
    Serial.println(response);
  } else {
    Serial.print("Failed to post data, response: ");
    Serial.println(response);
  }
   
  http.disconnect();
}

/*
 * setup
 */

void setup() {
  dht.begin();
  
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
  
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }

  DateTime now = rtc.now();
  setFilename(now);

  // Write line to indicate new measuring sequence
  logFile = SD.open(fout, FILE_WRITE);
  logFile.println("9999");
  logFile.close();

  Serial.println("initialization done.");
}


/*
 * loop
 */
 
void loop() {
  // put your main code here, to run repeatedly
  DateTime now = rtc.now();

  tsec = now.second();

  while ( tsec != tsecprev) {

    tsecprev = tsec;
    tmod = tsec % 30 ;
  
    if ( tmod == 0 ){
    
      readDHT();
      setDTstring(now);
            
      Serial.print(datetime);
      Serial.print(" - Temp (C): ");
      Serial.print(ftem);
      Serial.print(" - Hum (perc): ");
      Serial.println(fhum);
  
      writeOut(now);
  
      // Every 10 minutes a sample will be posted
      tmin = now.minute();
      while ( tmin != tminprev) {
    
        tminprev = tmin;
        tmod = tmin % 10 ;
      
        if ( tmod == 0 ){
          Serial.print("er wordt een sample verzonden... ");
          postReading();
        }
      }
    }
  } 
  // Wait 5 s before testing for the next 30 second interval
  delay(5000);
}
