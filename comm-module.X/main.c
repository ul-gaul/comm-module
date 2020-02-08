#include "main.h"


const char message[] = "hello PIC32";

int main(void) {
    int i;

	/* set PPS for UART1 to use pins D2 (Rx) and D3 (Tx) */
//    PB2DIV = 0x00ff;
	U1RXR = 0x00;
	RPD3R = 0x01;

	/* init UART1 bus with 115200 baudrate */
	U1MODEbits.ON = 1;
    U1STAbits.URXEN = 1;
    U1STAbits.UTXEN = 1;
    U1BRG = 13;
    
    i = 0;
	for(;;) {
		U1TXREG = message[i];
        i++;
        if (i >= 11) {
            i = 0;
        }
        while (U1STAbits.TRMT == 0);
	}
	
	return 0;
}
