/*
---------------------------------------------------------------------------
Ultrasonic Motion Detector Music Box
written for Arduino IDE 2.3.2 with ATTiny84 IC
7-30-24
---------------------------------------------------------------------------
*/

#include <NewPing.h>

#define TRIGGER_PIN  10  // Attiny84 Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9  // Attiny84 Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 450 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

// Pin assignments
const int led = 2;
const int motor = 8;
const int hauntedSelect = 0;
const int shortRangeMode = 1;

unsigned long hauntedInterval = 0; // How often the haunted function triggers in seconds
const int hauntedDuration = 5; // How long the haunted music plays when triggered in seconds
const int shortRangeDistance = 150; //Detection range of short range mode in centimeters

bool object_detected = false;
bool max_range = false; 
int sensorSamples = 0;
int sensorReading = 0;
unsigned long previousMillis = 0;
int randNumber;

void setup() {
  randomSeed(analogRead(7)); 
  hauntedInterval = random(600, 1200);

  pinMode(led, OUTPUT);
  pinMode(motor, OUTPUT);
  pinMode(hauntedSelect, INPUT);
  pinMode(shortRangeMode, INPUT);

  // if Short Range Switch is on then set the short detection range
  if(digitalRead(shortRangeMode)){	
    sensorSamples = shortRangeDistance;
  }

  // If Short Range Switch is off then sample and calibrate the range
  else{
	    // Take range samples to determine range to nearest object in front of sensor
    int i;
    for (i = 0; i < 10; i++) {
      digitalWrite(led, !digitalRead(led));   // toggles LED on and off while calibrating range
      sensorSamples = sensorSamples + sonar.ping_cm();
      delay(200);
    }
    digitalWrite(led, LOW);

    // Find the average of all samples.
    // 5 is subtracted to give a buffer to compensate for minor reading variations
    sensorSamples = (sensorSamples / 10) - 5; 
    
    if(sensorSamples <= 0){   // the sensor will read 0 if it does not detect a surface
      sensorSamples = 0;
      max_range = true;
    }
  }
}

void loop() {
  delay(50); //Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  
  //save sensor reading to a variable for later if not in max range mode
	if(!max_range){
    sensorReading = sonar.ping_cm();
  }
   
  //if the Music Box is facing an open area and returned a range of zero then trigger if anything is sensed. 
  if(max_range){
    if(sonar.ping_cm() > 5){
      playMusic(2);
    }
  }

  //if not at max range then compare new readings to the calibration value
  else if(sensorReading < sensorSamples && sensorReading > 0){
    playMusic(2);
  }

  //If the Haunted Mode is on wait ten minutes plus an addition random amount of time
  //and then turn on the music box for a short time
  if(digitalRead(hauntedSelect)){
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= (hauntedInterval * 1000)){ //Convert hauntedInterval into milliseconds
      previousMillis = currentMillis;
      hauntedInterval = random(600, 1200);
      playMusic(hauntedDuration);
    }
  }
}

void playMusic(int x){  //Turns on the music box motor for x seconds 
  digitalWrite(motor, HIGH);
  for (int i = 0; i < x; i++) {   //Loop and toggle LED every 1 second
    digitalWrite(led, !digitalRead(led));
    delay(1000);
  }

  //save sensor reading to a variable for later if not in max range mode
  sensorReading = sonar.ping_cm();
  
  if(max_range){
    while(sonar.ping_cm() > 5){
      digitalWrite(led, !digitalRead(led));
      delay(1000);
    }
  }
  else {
    //Check if the sensor is still blocked and if so keep playing
    while(sensorReading < sensorSamples && sensorReading > 0){
      digitalWrite(led, !digitalRead(led));
      delay(1000);
      sensorReading = sonar.ping_cm();
    }
  }
  
  digitalWrite(motor, LOW);
  digitalWrite(led, LOW);
}