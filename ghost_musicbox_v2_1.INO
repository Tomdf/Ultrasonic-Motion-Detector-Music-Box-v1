/*
---------------------------------------------------------------------------
Ultrasonic Motion Detector Music Box
written for Arduino IDE 2.3.2
7-5-24
---------------------------------------------------------------------------
*/

#include <NewPing.h>

#define TRIGGER_PIN  10  // Attiny84 Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     9  // Attiny84 Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

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
unsigned long previousMillis = 0;
int randNumber;

void setup() {
  randomSeed(analogRead(7)); 
  hauntedInterval = random(600, 1200);

  pinMode(led, OUTPUT);
  pinMode(motor, OUTPUT);
  pinMode(hauntedSelect, INPUT);
  pinMode(shortRangeMode, INPUT);

  if(digitalRead(shortRangeMode)){	// if Short Range Switch is on then set the short detection range
    sensorSamples = shortRangeDistance;
  }
  else{
	// If Short Range Switch is off then sample and calibrate the range
    // Take range samples to determine range to nearest object in front of sensor
    int i;
    for (i = 0; i < 10; i++) {
      digitalWrite(led, !digitalRead(led));   // toggles LED on and off while calibrating range
      sensorSamples = sensorSamples + sonar.ping_cm();
      delay(200);
    }
    digitalWrite(led, LOW);

    // Find the average of all samples.
    // 10 is subtracted to give a buffer to compensate for minor reading variations
    sensorSamples = (sensorSamples / 10) - 10; 
    
    if(sensorSamples <= 0){   // the sensor will read 0 if it does not detect a surface
      max_range = true;
    }
  }
}

void loop() {
  delay(50); //Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.

  if(max_range){
    if(sonar.ping_cm() > 10){
      playMusic(2);
    }
  }
  else if(sonar.ping_cm() < sensorSamples){
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

  //Check if the sensor is still blocked and if so keep playing
  while(sonar.ping_cm() < sensorSamples){
    digitalWrite(led, !digitalRead(led));
    delay(1000);
  }
  
  digitalWrite(motor, LOW);
  digitalWrite(led, LOW);
}