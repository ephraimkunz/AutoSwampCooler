//AutomatedSwampCooler controller
// Author: Ephraim Kunz

#include "DHT.h"

#define external_temp_pin 2
#define internal_temp_pin 3

#define low_blower_pin 8
#define high_blower_pin 9
#define pump_pin 10

#define set_temp 70 // Temperature we set on the interface to the circuit
boolean setupHappened = false; // Have we run setup? Use to change logging for when we are just setting things up.

#define temp_poll_interval (1000 * 10) // in milliseconds


typedef enum {
  low_blower_type,
  high_blower_type,
  pump_type
} relay_type;

// Order matters here, as relay_hot is actually low asserted logic.
typedef enum {
  relay_hot,
  relay_cold
} relay_state;

char *stringForState(relay_state state) {
  if (state == relay_hot) {
    return "on";
  }

  return "off";
}

struct relay {
  char *name;
  relay_type type;
  int pin;
  relay_state state;
};

relay low_blower = { "Low blower", low_blower_type, low_blower_pin, relay_cold };
relay high_blower = { "High blower", high_blower_type, high_blower_pin, relay_cold };
relay pump = { "Pump", pump_type, pump_pin, relay_cold };

#define DHTTYPE DHT11   // DHT 11 temperature sensors
DHT internal_temp  = DHT(internal_temp_pin, DHTTYPE);
DHT external_temp = DHT(external_temp_pin, DHTTYPE);

void logRelayChange(struct relay *theRelay, relay_state newState) {
  if(newState == theRelay->state) {
    Serial.print(theRelay->name);
    Serial.print( " stayed ");
    Serial.println(stringForState(newState));
  } else {
    Serial.print(theRelay->name);
    Serial.print( " changed from ");
    Serial.print(stringForState(theRelay->state));
    Serial.print(" to ");
    Serial.println(stringForState(newState));
  }
}

void setRelay(struct relay *theRelay, relay_state state){
  if (setupHappened)
    logRelayChange(theRelay, state);
  digitalWrite(theRelay->pin, state); 
  theRelay->state = state;
}

void initRelays(){
 pinMode(low_blower_pin, OUTPUT);
 pinMode(high_blower_pin, OUTPUT);
 pinMode(pump_pin, OUTPUT);

 // Everything off
 setRelay(&low_blower, relay_cold);
 setRelay(&high_blower, relay_cold);
 setRelay(&pump, relay_cold);
}

void logTemps(float in, float out) {
  Serial.print("Out: ");
  Serial.print(out);
  Serial.print(" *F, In: ");
  Serial.print(in);
  Serial.println(" *F");
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  
  Serial.println("Initializing relays ...");
  initRelays();
  Serial.println("Done");
  
  Serial.println("Initializing temperature sensors ...");
  internal_temp.begin();
  external_temp.begin();
  Serial.println("Done");
  
  Serial.println("Starting sketch");
  Serial.println();
  setupHappened = true;
}

void loop() {
  delay(temp_poll_interval);
  
  float out_temp = external_temp.readTemperature(true); // Read Farenheight
  float in_temp = internal_temp.readTemperature(true);
  logTemps(in_temp, out_temp);
  
  // Dad hates high fan, but other may like it so we will keep it defined in setup() still.
  if (out_temp > in_temp) {
    if(in_temp > set_temp) {
      // Pump + low fan
      setRelay(&high_blower, relay_cold);
      setRelay(&low_blower, relay_hot);
      setRelay(&pump, relay_hot);
    } else {
      // Turn off everything
      setRelay(&high_blower, relay_cold);
      setRelay(&low_blower, relay_cold);
      setRelay(&pump, relay_cold);
    }
  } else {
    // Low fan only
    setRelay(&high_blower, relay_cold);
    setRelay(&low_blower, relay_hot);
    setRelay(&pump, relay_cold);
  }
  Serial.println();
}
