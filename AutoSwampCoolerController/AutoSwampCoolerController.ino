//AutomatedSwampCooler controller
// Author: Ephraim Kunz

#include "DHT.h"

#define external_temp_pin 4
#define internal_temp_pin 5

#define low_blower_pin 8
#define high_blower_pin 9
#define pump_pin 10

#define set_temp 45 // Temperature we set on the interface to the circuit
#define margin_of_error 0.5 // Number of degrees F we will allow temps to be out of sync before changing things (to prevent turning on/off too often)
#define num_temp_reads_per_round 10 // More is better for averaging, but takes longer
#define delay_between_temp_reads 500 // In milliseconds. Thus, total time to read temp for a round is num_temp_reads_per_round * (delay_between_temp_reads + DHT read delay)
boolean setupHappened = false; // Have we run setup? Use to change logging for when we are just setting things up.

#define temp_poll_interval (1000 * 10) // in milliseconds: how often we poll for new temps / possible toggle relays


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

// Returns true if first is hotter than second, within the margin of error. Returns false otherwise
boolean isHotterThan(float first, float second) {
 float diff = first - second;
 return first > 0 && second > 0 && diff > 0 && diff > margin_of_error;
}

void getAveragedTemperatures(float *in, float *out) {
  float accum_in = 0.0;
  float accum_out = 0.0;
  for(int i = 0; i < num_temp_reads_per_round; ++i) {
    accum_in += internal_temp.readTemperature(true); // Read Farenheit
    accum_out += external_temp.readTemperature(true);
    delay(delay_between_temp_reads);
  }
  *in = accum_in / num_temp_reads_per_round;
  *out = accum_out / num_temp_reads_per_round;
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
  float out_temp;
  float in_temp;

  getAveragedTemperatures(&in_temp, &out_temp);
  logTemps(in_temp, out_temp);
  
  // Dad hates high fan, but other may like it so we will keep it defined in setup() still.
  if (isHotterThan(out_temp, in_temp)) {
    if(isHotterThan(in_temp, set_temp)) {
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
