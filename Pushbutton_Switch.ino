/****************************************************************************

  =========================================
  =  Push button toggle for load control  =
  =========================================

  Copyright 2022 Michael Kwun

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

****************************************************************************/

/*
   A momentary-on pushbutton is debounced in software.  Every button press
   toggles the state of a controlled load.

   The load is controlled using a 5v relay board using a LOW trigger and the 
   NC terminals.  This way, when the relay board is not powered, the load is 
   connected.  When the relay board is powered, then a LOW signal triggers 
   the relay, and the load is then disconnected.  The NC terminals are wired 
   to the high side, so with a LOW signal the load is not powered even if 
   there is a short to ground.
   
   Optionally, the load can also be controlled on the low side with a PWM
   signal.  This can be used for, e.g., speed control (if the load is a fan)
   or dimming (if the load is a light).  To omit this option, directly
   connect the low side of the load to ground.  To use this option, connect
   the low side through, e.g., a MOSFET to ground with the gate connected to
   the PWM pin.

*/

/* constants */

// pins
const byte pinB = 2;  // pushbutton
const byte pinL = 3;  // load relay trigger (high side control)
const byte pinD = 5;  // PWM (low side control)

// ms delay for debouncing
const unsigned long debounceDelay = 50;

// ms threshold between short/long presses
const unsigned long shortLong = 500;

// array for PWM levels, with initial duty in first slot
const byte pwm[] = {255, 46, 1, 46};
const byte sizeP = 4;

/* variables */

byte buttonState = HIGH;        // current button state
byte buttonProcessed = HIGH;    // last processed button state
unsigned long lastChange = 0;   // last change time
unsigned long buttonTime = 0;   // last debounced change time
byte duty = 0;                  // index for PWM array
byte col;                       // track col in monitor for dots

void setup() {
  pinMode(pinB, INPUT_PULLUP);  // momentary-on pushbutton
  pinMode(pinL, OUTPUT);        // trigger for load relay
  pinMode(pinD, OUTPUT);        // PWM signal for load
  analogWrite(pinD, pwm[duty]);
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println();
  Serial.println();
  Serial.println("*** BEGIN pushbutton toggle for load control.");
  Serial.println("Press to toggle load.");
  Serial.print("Long press when load is on changes duty cycle.");
  col = 46;
  // we end with print() not println() to allow trailing dots.
} /* end setup() */

/*
   In our main loop, we check to see if the button has changed state.  If it
   has, we debounce by waiting until the button has settled into its new
   state.  Once it does so, we take appropriate action, distinguishing
   between short and long button presses.

*/
void loop() {

  // if reported state of button changes note new state and time
  if (buttonState != digitalRead(pinB)) {
    buttonState ^= 1;
    lastChange = millis();
  }

  // if buttonState has not changed, check if we're outside bounce window
  else if ((millis() - lastChange) > debounceDelay) {

    // debounced buttonState just went up or down
    if (buttonProcessed != buttonState) {

      // button up event (due to internal pullup, up == HIGH)
      if (buttonState) {

        Serial.println();
        Serial.print("Button up: ");  // 11 characters

        // if load off, turn on load for short or long press
        if (!digitalRead(pinL)) {
          digitalWrite(pinL,HIGH);
          Serial.print("load off.");
          col = 20;
        }
        // if load on and short press, turn off load
        else if (((millis() - buttonTime) < shortLong)) {
          digitalWrite(pinL,LOW);
          Serial.print("short press; load off.");
          col = 33;
        } 
        // long press when load is on, change duty
        else {
          duty = (duty + 1) % sizeP;
          analogWrite(pinD, pwm[duty]);
          Serial.print("long press; level ");
          Serial.print(pwm[duty]);
          Serial.print(".");
          col = 31;
          if (pwm[duty] > 9) col++;
          if (pwm[duty] > 99) col++;
        }
        
      } /* end button up */

      // button was just pressed down
      else {
        Serial.println();
        Serial.print("Button down.");
        col = 12;
      } /* end button down */

      // note that we've processed this change in buttonState and time
      buttonProcessed = buttonState;
      buttonTime = lastChange;

    } /* end buttonState change */

    // Update buttonTime periodically where it is safely a long press already.
    // We do this to avoid a rare corner condition where, e.g., the button is 
    // untouched for ~50 days and the next press happens to fall within what 
    // would erroneously be treated as switch bounce, delaying action
    // momentarily.
    else if ((millis() - buttonTime) > (4 * shortLong)) {
      buttonTime += 4 * shortLong;
      if (col > 75) {
        Serial.println();
        Serial.print(".");
        col = 1;
      }
      else {
        Serial.print(" .");
        col += 2;
      }
    } /* end steady state */

  } /* end debounced button processing */

} /* end loop() */
