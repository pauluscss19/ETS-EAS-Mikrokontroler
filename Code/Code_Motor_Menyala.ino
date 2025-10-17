#include <Arduino.h>

int motorPin1 = 27;
int motorPin2 = 26;
int enablePin = 12;

const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(enablePin, OUTPUT);

  ledcAttach(enablePin, PWM_FREQ, PWM_RESOLUTION);

  // Set ke 0 untuk STOP
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  ledcWrite(enablePin, 0);
}

void loop() {
  // Motor berhenti
}