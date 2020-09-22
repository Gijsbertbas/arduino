/*
 * Sensor helper functions for the current_logger
 */

void readCurrent() {

  sensorValue = corf * analogRead(sensorPin);

  if (isnan(sensorValue)) {
    int count = 0;
    while (isnan(sensorValue) && count < 10) {
      delay(500);
      sensorValue = corf * analogRead(sensorPin);
      count += 1;
    }
  }
  fcur = calf * sensorValue;
  // fcur = calf * (roundf(100 * (sensorValue / YDHC)) / 100); //afronden stroom op 2 decimalen
}
