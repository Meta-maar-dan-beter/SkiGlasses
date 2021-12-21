#include <math.h>

// Rotary Encoder Inputs
#define CLK 0
#define DT 2
#define SW 14

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

int solar = 50;

void setup() {
  
  // Set encoder pins as inputs
  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Setup Serial Monitor
  Serial.begin(9600);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
}

// calculate the opaqueness of the glass
// parameters: solar level (0-100), manual setting (0-100)
// returns: value between 0 (fully tranparent) and 100 (fully opaque)
int calc_shade(int solar, int manual) {
    double exponent = pow(5.0, 1.0 - manual / 50.0);
    return (int)round(100.0 * pow(solar / 100.0, exponent));
}

void loop() {
  
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter --;
      currentDir ="CCW";
    } else {
      // Encoder is rotating CW so increment
      counter ++;
      currentDir ="CW";
    }

    int value = calc_shade(solar, counter);

    Serial.print("Solar: ");
    Serial.print(solar);
    Serial.print(" | Counter: ");
    Serial.println(counter);
    Serial.print("Formula: ");
    Serial.println(value);

    
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 50) {
      Serial.println("Button pressed!");
    }

    // Remember last button press event
    lastButtonPress = millis();
  }

  // Put in a slight delay to help debounce the reading
  delay(1);
}
