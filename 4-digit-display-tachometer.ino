#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <TimerOne.h>
#include <TM1637.h>

const byte CLK = 5;  // 4-digit display CLK pin 
const byte DIO = 6;  // 4-digit display DIO pin
const byte FAN_PIN = 10;  //PWM pin
const byte INT_PIN = 3;  // interrupt pin

const int MIN_DUTY = 0; // 100% speed
const int MAX_DUTY = 80; // 20% speed
const int INIT_SPEED = 20;  // initial speed is 20%

volatile unsigned int counter = 0;  // interrupts counter
unsigned int counter_buf = 0;       // stores current counter value while calculating
unsigned long curr_millis;         // current millis
unsigned long prev_millis;         // previous millis
const unsigned long norm_cycle_time = 30000;  // 60 (seconds) * 1000 (milliseconds) / 2 ticks per cycle
float cycle_time;
float rpm = 0;
float incoming_speed;

TM1637 tm(CLK,DIO);

void countup() {
  counter++;
}

void adjust_duty(){
  incoming_speed = Serial.parseFloat();
  if (incoming_speed >= 100 - MAX_DUTY & incoming_speed <= 100 - MIN_DUTY){
    Timer1.setPwmDuty(FAN_PIN, calc_duty(incoming_speed));
    Serial.print("speed set: ");
    Serial.println(incoming_speed, DEC);
  }
}

int calc_duty(float p){
  return ((100.0 - p )/ 100) * 1023;
}

void measure(){
  curr_millis = millis();
  counter_buf = counter;
  counter = 0;
  cycle_time = float(curr_millis - prev_millis);
  if (cycle_time > 0){      // timer number (curr_millis) will overflow (go back to zero), after approximately 50 days.
    rpm = (counter_buf/cycle_time)*norm_cycle_time;
    }
  //tm.display(rpm);
  prev_millis = curr_millis;
  Serial.print("revs: ");
  Serial.println(counter_buf, DEC);
}


void setup() {
  Serial.begin(9600);
  Timer1.initialize(40);  // 40 us = 25 kHz
  Timer1.pwm(FAN_PIN, calc_duty(INIT_SPEED));
  tm.init();
  tm.set(3);
  pinMode(INT_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(INT_PIN), countup, RISING);
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
