#include "main.h"


/* public functions */
int motor_control_send(uint8_t* src, unsigned int size);


/* global variables */
CommandPacket cmdpkt;
uint8_t serial_buf[32];


int main(void) {
	unsigned int size;

	/* set PPS for UART1 to use pins D2 (Rx) and D3 (Tx) */
	U1RXR = 0x00;
	RPD3R = 0x01;

	/* init UART1 bus with 115200 baudrate */
	U1MODEbits.ON = 1;
	U1STAbits.URXEN = 1;
	U1STAbits.UTXEN = 1;
	U1BRG = 13; // TODO adjust this value to get the baudrate


	/* (test) set attributes for command packet */
	cmdpkt.start_char = COMMAND_START;
	cmdpkt.function = 1;
	cmdpkt.arg = 1;
	cmdpkt.crc = 0;

	size = serialize_command_packet(&cmdpkt, serial_buf);

	/* main loop */
	for(;;) {
		motor_control_send(serial_buf, size);
	}

	return 0;
}


int motor_control_send(uint8_t* src, unsigned int size) {
	unsigned int i;

	for (i = 0; i < size; ++i) {
		U1TXREG = src[i];
		/* wait for TX buffer to be empty */
		while(U1STAbits.TRMT == 0);
	}

	return i;
}
