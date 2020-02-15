#include "main.h"


const char message[] = "hello PIC32";

int main(void) {
    int i;

	/* set PPS for UART1 to use pins D2 (Rx) and D3 (Tx) */
//    PB2DIVbits.PBDIV = 0x0001;
//    PB2DIVbits.ON = 1;
//
//	U1RXR = 0x00;
//	RPD3R = 0x01;

	/* init UART1 bus with 115200 baudrate */
//	U1MODEbits.ON = 1;
//    U1STAbits.URXEN = 1;
//    U1STAbits.UTXEN = 1;
//    U1BRG = 13;
    
    i = 0;
    TRISDbits.TRISD3 = 0;
	for(;;) {
//		U1TXREG = message[i];
//        i++;
//        if (i >= 11) {
//            i = 0;
//        }
//        while (U1STAbits.TRMT == 0);
//        U1TXREG = 0x55;
        
        LATDbits.LATD3 = 0;
        LATDbits.LATD3 = 1;
        
	}
	
	return 0;
}
