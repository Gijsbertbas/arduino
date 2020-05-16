/* 15 dec 17 DW 2337h VER 0.1.0
 created dec 2017 en aangepast jan 2018 by dick w
 adapted feb 2018 by gijs 
 rewrite jan 2019 by gijs:
  - reset clock at setup
  - combine temp and hum in one logfile
  - new logfile every day
  - post samples to server
 change apr 2019 by gijs:
  - restart sim every time for a new post
  - reset RTC using network time

current version: 0.7.0
 */
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SDM.h>
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

const int simPin =  45;
unsigned int RX_PIN = 52;
unsigned int TX_PIN = 53;
unsigned int RST_PIN = 12;
const char BEARER[] PROGMEM = "internet";
HTTP http(9600, RX_PIN, TX_PIN, RST_PIN); 

RTC_DS1307 rtc;
File logFile;

float fhum, ftem;
int tsec, tsecprev, smod, tmin, tminprev, mmod;
char fout[] = "yyyymmdd.log"; // filename max 8 char, ext max 3
char datetime[] = "yyyy-mm-dd hh:mm:ss";

/*
 * functions
 */

void setRTC(char* dt, char* tm) {
  // split date to fields in a datematrix (dm)
  int i=0;
  int dm[6];
  char* datesplit = strtok(dt, "/");
  while (datesplit != NULL)
  {
    dm[i] = atoi(datesplit); 
    i++;
    datesplit = strtok (NULL, "/");
  }
  // split time to fields
  char* timesplit = strtok(tm, ":");
  while (timesplit != NULL)
  {
    dm[i] = atoi(timesplit); 
    i++;
    timesplit = strtok (NULL, ":");
  }
  rtc.adjust(DateTime(dm[0],dm[1],dm[2],dm[3],dm[4],dm[5]));
}

void resetRTC() {
  digitalWrite(simPin, HIGH);
  delay(5000); // enough time to find network?

  Serial.println("trying to setup an internet connection");
  http.connect(BEARER);
  
  Serial.println("Calling function to get location and time...");
  char response[128];
  Result result = http.gettime(response);
  
  Serial.print("success code : ");
  Serial.println(result);
  Serial.print("response: ");
  Serial.println(response);

  // split response to fields
  int i=0;
  char matrix[5][11];
  char* split = strtok(response, ",");
  while (split != NULL)
  {
    strcpy(matrix[i],split); 
    Serial.println(matrix[i]);
    i++;
    split = strtok (NULL, ",");
  }

  http.disconnect();
  delay(2000); // wait 2 sec to allow complete disconnect
  digitalWrite(simPin, LOW);

  setRTC(matrix[3], matrix[4]);
}

void readDHT() {
  fhum = dht.readHumidity();
  ftem = dht.readTemperature();
  if ( isnan(fhum) || isnan(ftem)) {
    int count = 0;
    while ( (isnan(fhum) || isnan(ftem)) && count < 10) {
      delay(500);
      fhum = dht.readHumidity();
      ftem = dht.readTemperature();
      count += 1;
    }
  }
}

void setFilename(DateTime now) {
  sprintf(fout, "%02d%02d%02d.log", now.year(), now.month(), now.day());
  if (SD.exists(fout)) {
  } else{
    logFile = SD.open(fout, FILE_WRITE);
    logFile.println("timestamp temperature humidity");
    logFile.close();
    if (SD.exists(fout)) {
      Serial.print(fout);
      Serial.println(" created.");
    } else {
      Serial.println("failed to create logfile!");
    }
  }
}

void setDTstring(DateTime now) {
  sprintf(datetime, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}

void writeOut(DateTime now) {

  setFilename(now);
  
  logFile = SD.open(fout, FILE_WRITE);
  logFile.print("\"");
  logFile.print(datetime);
  logFile.print("\" ");
  logFile.print(ftem);
  logFile.print(" ");
  logFile.println(fhum);
  logFile.close();
}

void postReading() {

  digitalWrite(simPin, HIGH);
  
  char stem[6];
  char shum[6];
  dtostrf(ftem, 4, 2, stem);
  dtostrf(fhum, 4, 2, shum);

  char body[29];
  sprintf(body, BODY_FORMAT, stem, shum);
  delay(2000);
  Serial.println(body);

  http.connect(BEARER);

  // to allow the connection to fully establish, may not be needed:
  delay(1000); 
 
  char response[256];
  Result result;
  //result = http.get(GETENDPOINT, response);
  result = http.post(POSTENDPOINT, body, response);

  if (result == SUCCESS) {
    Serial.println("Post success!: ");
    Serial.println(response);
  } else {
    Serial.println("Failed to post data, response: ");
    Serial.println(response);
  }
   
  http.disconnect();
  delay(2000); // wait 2 sec to allow complete disconnect
  digitalWrite(simPin, LOW);

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

  // THE NEXT LINE HAS CHANGED FOR THE MEGA
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
    smod = tsec % 30 ;
  
    if ( smod == 0 ){
    
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
        mmod = tmin % 10 ;
      
        if ( mmod == 0 ){
          Serial.println("er wordt een sample verzonden... ");

          postReading();

        }
      }
    }
  } 
  // Wait 0.5 s before testing for the next 30 second interval
  delay(500);
}
