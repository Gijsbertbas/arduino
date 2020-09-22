/*
 * Output (file and api) helper functions for the current_logger
 */
 
void setFilename(DateTime now) {
  sprintf(fout, "%02d%02d%02d.log", now.year(), now.month(), now.day());
  if (SD.exists(fout)) {
  } else{
    logFile = SD.open(fout, FILE_WRITE);
    logFile.println("timestamp current");
    logFile.close();
    if (SD.exists(fout)) {
      Serial.print(fout);
      Serial.println(" created.");
    } else {
      Serial.println("failed to create logfile!");
    }
  }
}

void writeOut(DateTime now) {

  setFilename(now);

  dtostrf(fcur, 4, 2, scur);

  sprintf(logstring, "\"%s\" %s", datetime, scur);

  logFile = SD.open(fout, FILE_WRITE);
  logFile.println(logstring);
  logFile.close();
}

void postReading() {

  digitalWrite(simPin, HIGH);

  dtostrf(fcur, 4, 2, scur);

  char body[29];
  sprintf(body, BODY_FORMAT, scur);
  delay(2000);
  Serial.println(body);

  http.connect(BEARER);

  // to allow the connection to fully establish, may not be needed:
  delay(1000);

  char response[256];
  Result result;
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
