# AutoSwampCooler
Arduino and Raspberry Pi code to automate your swamp cooler (turn on the low blower when it is cooler outside than inside)

## Required Parts
* (2) DHT-11 Temperature/Relative Humidity sensors
* (1) At least a 3 channel relay board. I used <a href="https://www.amazon.com/gp/product/B00KTEN3TM/ref=oh_aui_detailpage_o02_s00?ie=UTF8&psc=1">this</a>.
* Various jumpers, 10k ohm resistors, thermostat wire, and 12 or 14 gauge 120 V wire
##

## Dependencies
* To read from the temperature sensors, we will be using the Adafruit <a href="https://github.com/adafruit/DHT-sensor-library">DHT Sensor Library</a>. Put this in the libraries folder of your Arduino environment by following the instructions in that project's README.
