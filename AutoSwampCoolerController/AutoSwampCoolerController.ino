//AutomatedSwampCooler controller
// Written by Ephraim Kunz

#include "DHT.h"

#define ExtTempSens 2
#define IntTempSens 3

#define AutoSwitch 4
#define LowBlowerSwitch 5
#define HighBlowerSwitch 6
#define PumpSwitch 7

#define RelayLowBlower 8
#define RelayHighBlower 9
#define RelayPump 10

const unsigned long TimeBetweenAverages = 1000 * 15; //30 seconds
const unsigned long TimeBetweenSamples = 1000 * 2;
unsigned long lastAverage = 0;
int numDataPoints = 0;
double runningInSum = 0;
double runningOutSum = 0;
const double TempDiffThreshold = 2; //Difference of two degrees Farenheit before we do anything

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dhtIn  = DHT(IntTempSens, DHTTYPE);
DHT dhtOut = DHT(ExtTempSens, DHTTYPE);

/******** Switches *************/

void initSwitches(){
 pinMode(AutoSwitch, INPUT);
 pinMode(LowBlowerSwitch, INPUT);
 pinMode(HighBlowerSwitch, INPUT);
 pinMode(PumpSwitch, INPUT);
}
boolean automaticModeEnabled(){
  return digitalRead(AutoSwitch) == LOW;
}

/********* Relays ********/
void setRelay(int relayPin, int newState){
 digitalWrite(relayPin, newState); 
}

void initRelays(){
 pinMode(RelayHighBlower, OUTPUT);
 pinMode(RelayLowBlower, OUTPUT);
 pinMode(RelayPump, OUTPUT)
}

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
  if(lowBlowerCurrentState == LOW){
    Serial.println("Low blower remains running");
  }
  else{
    Serial.print("Low blower was off for ");
    Serial.print((millis() - lowBlowerStateChangeTime) / 1000.0);
    Serial.println(" seconds");
    Serial.println("Starting low blower ...");
    digitalWrite(RelayLowBlower, LOW);
    lowBlowerCurrentState = LOW;
    lowBlowerStateChangeTime = millis();
  }
}


void stopLowBlower(){
  if(lowBlowerCurrentState == HIGH){
    Serial.println("Low blower remains stopped");
  }
  else{
    Serial.print("Low blower ran for ");
    Serial.print((millis() - lowBlowerStateChangeTime) / 1000.0);
    Serial.println(" seconds");
    Serial.println("Stopping low blower ...");
    digitalWrite(RelayLowBlower, HIGH);
    lowBlowerCurrentState = HIGH;
    lowBlowerStateChangeTime = millis();
  }
}



void setup() {
  Serial.begin(9600);
  
  Serial.println();
  Serial.print("Initializing relays ...");
  initLowBlower();
  initSwitches();
  Serial.println("Done");
  
  Serial.print("Initializing temperature sensors ...");

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
  if(automaticModeEnabled()){
      delay(TimeBetweenSamples);
    
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
      
      if((avgIn - avgOut) >= TempDiffThreshold){
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
  
  else{ // We are in manual mode
    setRelay(RelayHighBlower, digitalRead(HighBlowerSwitch));
    setRelay(RelayLowBlower, digitalRead(LowBlowerSwitch));
    setRelay(RelayPump, digitalRead(PumpSwitch));
  }
}
