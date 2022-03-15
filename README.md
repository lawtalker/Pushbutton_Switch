# Pushbutton_Switch
I use this to control some LED strip lights with a pushbutton switch, but it could be used with other loads like a fan, too.  The pushbutton is debounced in software, and the code detects short versus long button presses.  Uses a low-voltage relay (to control the v+ connection on the load) and optionally a MOSFET (to control the v- connection on the load, via PWM).
