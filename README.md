# Arduino project files
This repository contains some sketches for arduino projects that I (have) work(ed) on.

## DHT logger
Logging temperature and humidity using a DHT22 sensor and a SIM800 module. The measured values are saved to an SD card every 30 seconds and sent to a server via HTTP every 10 minutes.

This project started with an [Arduino Uno](https://store.arduino.cc/arduino-uno-rev3) but as we ran into memory limitations we switched to a [Mega](https://store.arduino.cc/arduino-mega-2560-rev3). The last Uno sketch can be found [here](./sketchbook/uno/dht_logger), the latest and currently used script for the Mega is [here](./sketchbook/mega/dht_logger).

Samples are posted to a Django app hosted at Pythonanywhere where they are visualised in an interactive Bokeh plot, see [here an example](http://gbstraathof.pythonanywhere.com/arduino/apilog/dhtlogger01) for one of the loggers.

#### Requirements
The SD, Wire and SPI libraries should be installed with the Arduino IDE. For the RTC (clock) and the DHT we use the Adafruit libraries: [RTClib library](https://github.com/adafruit/RTClib) (find it in the IDE by searching for "RTClib by Adafruit") and [DHT library](https://github.com/adafruit/DHT-sensor-library) (search for "dht sensor library").

For the SIM800 module we use the excellent module by [Antonio Carrasco](https://github.com/carrascoacd/ArduinoSIM800L). We have forked it to add an additional functionality: we take the network time from the GPRS network to sync the RTC. See installation instructions in the [forked repository](https://github.com/Gijsbertbas/ArduinoSIM800L).

----

The plotting folder contains some (test) scripts used for generating the website figures. The at_commands folder contains some documentation on AT commands for the SIM800.

Don't hesitate to get in touch!

Gijs and Dick
