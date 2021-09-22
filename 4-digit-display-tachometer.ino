#include <avr/io.h>
#include <util/delay.h>
#include "TimerOne.h"
#include "PinChangeInterrupt.h"

#define PIN_TACH_0 A0 // PIN A0 PCINT8
#define INT_TACH_0 8

#define PIN_TACH_1 A1 // PIN A1 PCINT9
#define INT_TACH_1 9

#define PIN_TACH_2 A2 // PIN A2 PCINT10
#define INT_TACH_2 10

#define PIN_TACH_3 A3 // PIN A3 PCINT11
#define INT_TACH_3 11

const byte FAN_PIN = 10;  //PWM pin
const int MIN_DUTY = 0; // 100% speed
const int MAX_DUTY = 80; // 20% min speed
const int INIT_SPEED = 25;  // initial speed is 25%

volatile unsigned int counters[4] = {0, 0, 0, 0};  // interrupt counters
unsigned int counter_buf[4] = {0, 0, 0, 0};       // stores current counter value while calculating
unsigned long curr_micros;         // current micros
unsigned long prev_micros;         // previous micros
//unsigned long prev_micros_debug;
const unsigned long norm_cycle_time = 30000000;  // 60 (seconds) * 1000000 (microseconds) / 2 ticks per cycle
float cycle_time;
float rpm[4] = {0, 0, 0, 0};
float incoming_speed;

void setup() {
  Serial.begin(9600);
  Timer1.initialize(40);  // 40 us = 25 kHz
  Timer1.pwm(FAN_PIN, calc_duty(INIT_SPEED));
  pinMode(PIN_TACH_0, INPUT_PULLUP);
  pinMode(PIN_TACH_1, INPUT_PULLUP);
  pinMode(PIN_TACH_2, INPUT_PULLUP);
  pinMode(PIN_TACH_3, INPUT_PULLUP);

  attachPinChangeInterrupt(INT_TACH_0, countup_0, FALLING);
  attachPinChangeInterrupt(INT_TACH_1, countup_1, FALLING);
  attachPinChangeInterrupt(INT_TACH_2, countup_2, FALLING);
  attachPinChangeInterrupt(INT_TACH_3, countup_3, FALLING);
  sei();
}

void loop() {
  if (Serial.available() > 0) {
    adjust_duty();
  }
  measure();
  delay(2000);
}

void countup_0() {
  counters[0]++;
  //Serial.println(micros()-prev_micros_debug); //temporary debug stuff
  //prev_micros_debug=micros();
}

void countup_1() {
  counters[1]++;
  //Serial.println(micros()-prev_micros_debug);
  //prev_micros_debug=micros();
}

void countup_2() {
  counters[2]++;
  //Serial.println(micros()-prev_micros_debug);
  //prev_micros_debug=micros();
}

void countup_3() {
  counters[3]++;
  //Serial.println(micros()-prev_micros_debug);
  //prev_micros_debug=micros();
}

void adjust_duty() {
  incoming_speed = Serial.parseFloat();
  if (incoming_speed >= 100 - MAX_DUTY & incoming_speed <= 100 - MIN_DUTY) {
    Timer1.setPwmDuty(FAN_PIN, calc_duty(incoming_speed));
    Serial.print("speed set: ");
    Serial.println(incoming_speed, DEC);
  }
}

int calc_duty(float p) {
  return ((100.0 - p ) / 100) * 1023;
}

void measure() {
  curr_micros = micros();
  memcpy(counter_buf, counters, sizeof(counters));
  memset(counters,0,sizeof(counters));
  cycle_time = float(curr_micros - prev_micros);
  if (cycle_time > 0) {     // timer number (curr_micros) will overflow (go back to zero), after approximately 50 days.
    for(short i=0; i <4; i++){
      rpm[i] = (counter_buf[i]/cycle_time) * norm_cycle_time;
      }
  }
  prev_micros = curr_micros;
  for(short j=0; j<4; j++){
      Serial.print(counter_buf[j], DEC);
      Serial.print(" -> ");
      Serial.println(rpm[j], DEC);
    }
}
