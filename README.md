# comm-module
code for the communications module


# Motor control command protocol
The commands sent to the motor control unit are structured as follows:

- 2 start bytes: 0xface
- 1 function byte
- 1 argument byte
- 2 bytes for the 16-bit CRC

The function byte can take the values:

- 0: Sets the motor control unit in a particular state of the launch
	sequence. The argument is the launch state as defined in the 
	documentation for the motor control.
- 1: Overrides the state of an actuator. The argument is the state of
	the specified actuator, either 0 or 1, other values are ignored.

