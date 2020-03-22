#include "main.h"


/* global variables */
CommandPacket cmdpkt;
unsigned char serial_buf[32];
unsigned int i;


int main(void) {
	unsigned int size;

	/* set PPS for UART1 to use pins D2 (Rx) and D3 (Tx) */
	U1RXR = 0x00;
	RPD3R = 0x01;

	/* init UART1 bus with 115200 baudrate */
	U1MODEbits.ON = 1;
	U1STAbits.URXEN = 1;
	U1STAbits.UTXEN = 1;
	U1BRG = 82; /* 115200 baudrate */


	/* (test) set attributes for command packet */
	cmdpkt.start_char = COMMAND_START;
	cmdpkt.function = 1;
	cmdpkt.arg = 1;
	cmdpkt.crc = 0;

//	size = serialize_command_packet(&cmdpkt, serial_buf);

	/* main loop */
	for(;;) {
		cmdpkt.function = (i % 2 == 0) ? 1 : 2;
		cmdpkt.arg = i % 7;
		serialize_command_packet(&cmdpkt, serial_buf);
		motor_control_send(serial_buf, size);
	}

	return 0;
}


int init_motor_control_uart(void) {
	UART1_RX_PS();
	UART1_TX_PS();

	return 0;
}


int init_antenna_uart(void) {
	UART2_RX_PS();
	UART2_TX_PS();

	return 0;
}


int init_avionics_uart(void) {
	UART3_RX_PS();
	UART3_TX_PS();

	return 0;
}


int motor_control_send(uint8_t* src, unsigned int size) {
	unsigned int i;

	for (i = 0; i < size; ++i) {
		U1TXREG = src[i];
		/* wait for TX buffer to be empty */
		while (U1STAbits.TRMT == 0);
        while (U1STAbits.UTXBF == 1);
	}

	return i;
}
