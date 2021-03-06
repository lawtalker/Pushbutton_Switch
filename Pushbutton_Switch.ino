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

   The load (in this instance, 24v LED strip lights) could be controlled via 
   the high side (v+) or the low side (v-).  High side control has the 
   advantage of offering better protection against shorts, because the high
   side is probably hot, so by controlling the high side you avoid sending 
   current to the load in the event of a short.  Low side control has the 
   advantage of convenience, as it is easily controlled via MOSFET.

   The sketch offers logic for both types of control.  One option is a high-
   side control via a low-voltage relay board using a HIGH trigger and the 
   NC terminals.  (By using the NC terminals, push button toggle isn't 
   powered, then the load operations as an unswitched load.) A second option 
   is a MOSFET on the low side, which the sketch controls via a PWM-pin  
   This allows for dimming (for a light) or speed control (for a fan).  The 
   sketch offers a choice of several duty levels, which can be chosen by 
   long clicks while the load is on.

   Only one of these options needs to be wired up.  If you want a simple on-
   off switch, you can just wire up a relay on the high side of the load.  
   If you need dimming or PWM control, and you are not concerned about short 
   protection (e.g. if the voltage is low), you can just wire up the low 
   side with a MOSFET.  If you want to protect against a short in the load 
   and want to control the duty cycle, wire up both. 
*/

/* constants */

// pins
const byte pinB = 2;  // pushbutton
const byte pinL = 3;  // load relay trigger (high side control)
const byte pinD = 5;  // PWM (low side control)

// ms delay for debouncing button
const unsigned long debounceDelay = 50;

// ms threshold between short/long button presses
const unsigned long shortLong = 500;

// array for PWM duty levels, with initial duty in first slot
const byte duty[] = {255, 46, 1, 46};
const byte dutyS = 4;
/* variables */

byte buttonState = HIGH;        // raw button state
byte buttonProcessed = HIGH;    // processed button state
unsigned long lastChange = 0;   // last raw change time
unsigned long buttonTime = 0;   // last debounced change time
byte dutyI = 0;                 // index for PWM array
byte col;                       // column in monitor (for line wrapping)

void setup() {
  pinMode(pinB, INPUT_PULLUP);  // use external pullup for long wire run
  pinMode(pinL, OUTPUT);        // trigger for load relay
  pinMode(pinD, OUTPUT);        // PWM signal for load
  analogWrite(pinD, duty[dutyI]);

  Serial.begin(115200);
  digitalWrite(LED_BUILTIN,HIGH);
  delay(3000);
  digitalWrite(LED_BUILTIN,LOW);
  Serial.println("\r\n\r\n*** BEGIN pushbutton toggle for load control.");
  Serial.print("Load is on at ");
  Serial.print(duty[dutyI]);
  Serial.println(" duty.  Press button to toggle load.");
  Serial.print("Long press when load is on changes duty cycle.");
  col = 46;
  // we end with print() not println() to allow trailing dots.
  
} /* end setup() */

/*
   We check to see if the button has changed state.  If not, we take action 
   only if the button has settled into its new state (i.e. we debounce).  
   After debouncing, we take appropriate action, distinguishing between 
   short and long button presses.  
*/
void loop() {

  // raw button change (prior to debouncing)
  if (buttonState != digitalRead(pinB)) {
    buttonState ^= 1;
    lastChange = millis();
  }

  // software button debounce (do nothing for recent raw button changes)
  else if ((millis() - lastChange) > debounceDelay) {

    // debounced button change
    if (buttonProcessed != buttonState) {

      // button up (with pullup, up == HIGH)
      if (buttonState) {
        
        Serial.print("\r\nButton up: ");  // col 11
        
        // load on (when load off - trigger is HIGH)
        if (digitalRead(pinL)) {
          digitalWrite(pinL,LOW);
          analogWrite(pinD, duty[dutyI]);
          Serial.print("load on.");
          col = 20;
        }
        // load off (load on and short press)
        else if (((millis() - buttonTime) < shortLong)) {
          digitalWrite(pinL,HIGH);
          analogWrite(pinD, 0);
          Serial.print("short press; load off.");
          col = 33;
        } 
        // change duty (load on and long press)
        else {
          dutyI = (dutyI + 1) % dutyS;
          analogWrite(pinD, duty[dutyI]);
          Serial.print("long press; level ");
          Serial.print(duty[dutyI]);
          Serial.print(".");
          col = 31;
          if (duty[dutyI] > 9) col++;
          if (duty[dutyI] > 99) col++;
        }
        
      } /* end button up */

      // button down
      else {
        // just reporting; actions on button up
        
        Serial.print("\r\nButton down.");
        col = 12;
        
      } /* end button down */
      
      // debounced button change flag & time
      buttonProcessed = buttonState;
      buttonTime = lastChange;

    } /* end debounced button change */

    // keep (millis() - times) from overflowing 
    else if ((millis() - buttonTime) > (2 * shortLong)) {
      buttonTime += shortLong;
      lastChange += shortLong;
      if (col > 75) {
        Serial.print("\r\n.");
        col = 1;
      }
      else {
        Serial.print(" .");
        col += 2;
      }
    } /* end debounced button steady state */

  } /* end debounced button */

} /* end loop() */
