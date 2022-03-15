# Pushbutton_Switch
I use this to control some LED strip lights with a pushbutton switch, but it could be used with other loads like a fan, too.

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
