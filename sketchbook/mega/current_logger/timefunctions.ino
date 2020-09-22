/*
 * Clock and daytime helper functions for the current_logger
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

void setDTstring(DateTime now) {
  sprintf(datetime, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}

bool isDST(DateTime dt) {
  /*
   * Check if we passed the DST transition
   * The transition will be synchronous with the rtc adjustment...
   * ...once a day at midnight (effectively the first Monday after the official transition) 
   */
  bool dst;
  if(dt.month() == 3 && dt.day() - dt.dayOfTheWeek() > 24 && dt.dayOfTheWeek() > 0) dst = true;
  else if(dt.month() > 3 && dt.month() < 10) dst = true;
  else if(dt.month() == 10 && dt.day() - dt.dayOfTheWeek() > 24 && dt.dayOfTheWeek() > 0) dst = false;
  else if(dt.month() == 10) dst = true;
  else dst = false;
  return dst;
}

bool isDaytime(DateTime dt, Dusk2Dawn loc, bool dst, int lag) {
  /*
   * Check it daytime: between 'lag' minutes before sunrise and 'lag' minutes after sunset
   */
  int elapsed = dt.hour() * 60 + dt.minute();
  return elapsed > loc.sunrise(dt.year(), dt.month(), dt.day(), dst)-lag && elapsed < loc.sunset(dt.year(), dt.month(), dt.day(), dst)+lag;
}
