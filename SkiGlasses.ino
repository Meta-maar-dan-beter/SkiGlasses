#include <math.h>
#include "UV-sensor.h"

// Rotary Encoder Inputs
#define CLK 2
#define DT 3
#define SW 4

// PWM pin for output
#define PWM 5

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
  Serial.begin(9600);

  // Init UV sensor
  SI1145_init_sensor();
}

// calculate the opaqueness of the glass
// parameters: solar level (0-100), manual setting (0-100)
// returns: value between 0 (fully tranparent) and 100 (fully opaque)
int calc_shade(int solar, int manual) {
  double exponent = pow(5.0, 1.0 - manual / 50.0);
  int ret = round(100.0 * pow(solar / 100.0, exponent));
  if (ret == 0) ret = 1; // Make sure value never hits zero
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
  OCR1A = period * 15.625 - 1;
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts

}

ISR(TIMER1_COMPA_vect) {
  digitalWrite(PWM, HIGH);
  asm("nop\nnop\nnop\nnop\nnop\nnop\n"); // short delay
  digitalWrite(PWM, LOW);
}

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

    Serial.print("Solar: ");
    Serial.print(solar.vis);
    Serial.print(" | Counter: ");
    Serial.println(counter);
    Serial.print("Formula: ");
    Serial.println(value);

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
      Serial.println("Button pressed!");
    }

    // Remember last button press event
    lastButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  delay(1);
}
