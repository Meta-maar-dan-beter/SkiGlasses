#include <math.h>
#include "UV-sensor.h"

// Defs for Arduino UNO
#ifdef ARDUINO_AVR_UNO

#define DEBUG

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4

// PWM pin for output
#define PWM 5

#endif

// Defs for ATtiny84
#ifdef ARDUINO_attiny

// Rotary Encoder Inputs
#define CLK 8
#define DT 2
#define SW 3

// PWM pin for output
#define PWM 7

#endif


// Global variables
int counter = 50;
SI1145_value solar;
bool power = true;

void setup() {
  
  // Set encoder pins as inputs
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);

  // Set up hardware interrupt
  attachInterrupt(0, updateEncoder, FALLING);

  // Setup Serial Monitor
#ifdef DEBUG
  Serial.begin(9600);
#endif

  // Init UV sensor
  SI1145_init_sensor();
}

// calculate the opaqueness of the glass
// parameters: solar level (0-100), manual setting (0-100)
// returns: value between 0 (fully tranparent) and 100 (fully opaque)
int calc_shade(int solar, int manual) {
  double exponent = pow(5.0, 1.0 - manual / 50.0);
   int ret = round(100.0 * pow(solar / 100.0, exponent));
   if (ret < 1) ret = 1; // Make sure value never hits zero
   return ret;
}

// set timer 1 period in ms
void setInterrupt(int period) {
  cli();//stop interrupts

  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // set compare match register
  OCR1A = period * (F_CPU / 1024000.0); // Clock freq / 1024 (prescaler) / 1000 (ms)
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts

}

#ifdef ARDUINO_AVR_UNO
ISR(TIMER1_COMPA_vect) {
  digitalWrite(PWM, HIGH);
  asm("nop\nnop\nnop\nnop\nnop\nnop\n"); // short delay
  digitalWrite(PWM, LOW);
}
#endif

#ifdef ARDUINO_attiny
ISR(TIM1_COMPA_vect) {
  digitalWrite(PWM, HIGH);
  asm("nop\nnop\nnop\n"); // short delay
  digitalWrite(PWM, LOW);
}
#endif

void updateEncoder() {
  int dat = digitalRead(DT);
  if (dat == HIGH && counter < 100) counter++;
  if (dat == LOW && counter > 0) counter--;
  int value = calc_shade(solar.vis, counter); // use visual reading for testing, change to UV later
  if (power) setInterrupt(value);
}
void loop() {
  static unsigned long lastButtonPress = 0;
  static unsigned long lastSensorRead = 0;

  if (millis() - lastSensorRead > 1000) {
    solar = SI1145_read_sensor();
    int value = calc_shade(solar.vis, counter); // use visual reading for testing, change to UV later
    if (power) setInterrupt(value);

#ifdef DEBUG
    Serial.print("Solar: ");
    Serial.print(solar.vis);
    Serial.print(" | Counter: ");
    Serial.println(counter);
    Serial.print("Formula: ");
    Serial.println(value);
#endif

    lastSensorRead = millis();
  }

  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      power = !power;
      if (!power) setInterrupt(100);
#ifdef DEBUG
      Serial.println("Button pressed!");
#endif
    }

    // Remember last button press event
    lastButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  delay(1);
}
