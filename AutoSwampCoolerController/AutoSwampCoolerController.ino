//AutomatedSwampCooler controller
// Written by Ephraim Kunz

#include "DHT.h"

#define ExtTempSens 2
#define IntTempSens 3
#define RelayLowBlower 8
#define RelayHighBlower 5
#define RelayPump 6

const unsigned long TimeBetweenAverages = 1000 * 15; //30 seconds
unsigned long lastAverage = 0;
int numDataPoints = 0;
double runningInSum = 0;
double runningOutSum = 0;
const double TempDiffThreshold = 2; //Difference of two degrees Farenheit before we do anything

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dhtIn  = DHT(IntTempSens, DHTTYPE);
DHT dhtOut = DHT(ExtTempSens, DHTTYPE);

/******* Low Blower ***********/
boolean lowBlowerCurrentState;
unsigned long lowBlowerStateChangeTime;

void initLowBlower(){
  pinMode(RelayLowBlower, OUTPUT);
  digitalWrite(RelayLowBlower, LOW);
  lowBlowerCurrentState = LOW;
  lowBlowerStateChangeTime = millis();
}

void startLowBlower(){
  if(lowBlowerCurrentState == HIGH){
    Serial.println("Low blower remains running");
  }
  else{
    Serial.print("Low blower was off for ");
    Serial.print((millis() - lowBlowerStateChangeTime) / 1000.0);
    Serial.println(" seconds");
    Serial.println("Starting low blower ...");
    digitalWrite(RelayLowBlower, HIGH);
    lowBlowerCurrentState = HIGH;
    lowBlowerStateChangeTime = millis();
  }
}


void stopLowBlower(){
  if(lowBlowerCurrentState == LOW){
    Serial.println("Low blower remains stopped");
  }
  else{
    Serial.print("Low blower ran for ");
    Serial.print((millis() - lowBlowerStateChangeTime) / 1000.0);
    Serial.println(" seconds");
    Serial.println("Stopping low blower ...");
    digitalWrite(RelayLowBlower, LOW);
    lowBlowerCurrentState = LOW;
    lowBlowerStateChangeTime = millis();
  }
}



void setup() {
  Serial.begin(9600);
  
  Serial.println("Initializing relays ...");
  initLowBlower();
  Serial.println("Done");
  
  Serial.println("Initializing temperature sensors ...");

  dhtIn.begin();
  dhtOut.begin();
  Serial.println("Done");
}


/* For now, this is how we do timing:
1. Take temperature every two seconds on both sensors.
2. After TimeBetweenAverages miliseconds, average them.
3. If there is a difference of more than TempDiffThreshold degree F, turn on blower.
4. Else, turn off blower
*/

void loop() {
  delay(2000);
  
  float inHumid = dhtIn.readHumidity();
  float inTemp = dhtIn.readTemperature(true);
  
  float outHumid = dhtOut.readHumidity();
  float outTemp = dhtOut.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(inTemp) || isnan(outTemp)) {
    Serial.println("Failed to read from DHT sensors. ");
    Serial.println("Trying again...");
    return;
  }
  
  Serial.print("inTemp: ");
  Serial.print(inTemp);
  Serial.print("\toutTemp: ");
  Serial.print(outTemp);
  Serial.println();
  
  runningInSum += inTemp;
  runningOutSum += outTemp;
  numDataPoints ++;
  
  if(millis() > (lastAverage + TimeBetweenAverages)){
    double avgOut = runningOutSum / numDataPoints;
    double avgIn = runningInSum / numDataPoints;
    
    Serial.println();
    Serial.print("avgInTemp: ");
    Serial.print(avgIn);
    Serial.print("\tavgOutTemp");
    Serial.print(avgOut);
    Serial.println();
    
    if(abs(avgIn - avgOut) >= TempDiffThreshold){
      startLowBlower();
    }
    else{
     stopLowBlower(); 
    }
    
    Serial.println();
    lastAverage = millis();
    runningInSum = 0;
    runningOutSum = 0;
    numDataPoints = 0;
  }
  
}
