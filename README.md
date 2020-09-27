# Communications module

Code for the communications module. 

## Serial communication

The module uses the UART protocol to communicate to the avionics module,
the motor control unit and the ground station.

### Motor control UART link

The motor control UART bus used it the UART1 bus, at 115200 baud.
An interrupt is setup to trigger on reception of data.

### Ground station UART link

The link to the ground station consists of a UART link at 115200 baud,
going through a radio module in the 900 MHz band. The module transmits
through an 8" omnidirectional antenna and the ground station receives
using a Yagi-Uda antenna.
The UART bus used is the UART2 bus.

### Avionics module UART link

The avionics UART is the UART3 bus.
An interrupt should be set up, but currently isn't.

## CRC quirks

You may notice the CRC seed value `CRC_SEED` in `crc.h` is not what is
specified in the rocket packet documenation. This is because the normal
implementation of the CRC CCITT-16-False assumes the last 16 bits of the
message are `0`s and computes the CRC over them. The DMA module of the
PIC does not assume this, so the seed has to be changed to account for
this.

# Setting up an interrupt and interrupt handler on the PIC32MZ

For an interrupt to be set, a few this must be configured in the PIC's
registers.

1- The interrupt source must have its interrupt enable bit set,
	along with a priority and sub-priority set. Before enabling the interrupt,
	it is recommended to clear (set to 0) the interrupt flag.
2- Enable interrupts in the PIC, usually using multi-vector interrupts.
3- A function for the interrupt handler must be created, specifying which
	vector and shadow register is associated with the source.

