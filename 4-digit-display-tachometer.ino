#include "TM1637.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <TimerOne.h>

#include <TM1637.h>

const byte CLK = 5;
const byte DIO = 6;
const byte fanPin = 10;
const byte IntPin = 3;

const int minDuty = 0;
const int maxDuty = 80;
const int initialDuty = 20;

volatile unsigned int counter = 0;
unsigned int counter_buf = 0;
unsigned long curr_millis;         //current millis
unsigned long prev_millis;         //previous millis
const unsigned long norm_cycle_time = 30000;  // 60seconds * 1000 millis / 2 ticks per cycle
float cycle_time;
float rpm = 0;
unsigned int incoming_duty;

TM1637 tm(CLK,DIO);

void countup() {
  counter++;
}

void adjust_duty(){
  incoming_duty = Serial.parseInt();
  if (incoming_duty >= 100 - maxDuty & incoming_duty <= 100 - minDuty){
    Timer1.setPwmDuty(fanPin, calc_duty(incoming_duty));
    Serial.print("duty set: ");
    Serial.println(incoming_duty, DEC);
  }
}

int calc_duty(float p){
  return ((100.0 - p )/ 100) * 1023;
}

void measure(){
  curr_millis = millis();
  counter_buf = counter;
  Serial.print("revs: ");
  Serial.println(counter_buf, DEC);
  counter = 0;
  cycle_time = float(curr_millis - prev_millis);
  if (cycle_time > 0){      // timer number (curr_millis) will overflow (go back to zero), after approximately 50 days.
    rpm = (counter_buf/cycle_time)*norm_cycle_time;
    }
  //tm.display(rpm);
  prev_millis = curr_millis;
}


void setup() {
  Serial.begin(9600);
  Timer1.initialize(40);  // 40 us = 25 kHz
  Timer1.pwm(fanPin, calc_duty(initialDuty));
  tm.init();
  tm.set(3);
  pinMode(IntPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(IntPin), countup, RISING);
  sei();
}

void loop() {
  if (Serial.available() > 0){
      adjust_duty();
    }
  tm.display(int(rpm));
  Serial.println(rpm);
  measure();
  delay(2000);
}
