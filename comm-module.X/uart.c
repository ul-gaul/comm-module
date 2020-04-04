#include "uart.h"


int init_motor_control_uart(void) {
	UART1_RX_PS();
	UART1_TX_PS();

	/* init UART1 bus with 115200 baudrate */
	U1MODEbits.ON = 1;
	U1STAbits.URXEN = 1;
	U1STAbits.UTXEN = 1;
	U1BRG = 82;

	/*
	 * UART1 interrupts:
	 * enable RX interrupt
	 * priority = 1, sub-priority = 3
	 */
	IEC3bits.U1RXIE = 1;
	IPC28bits.U1RXIP = 1;
	IPC28bits.U1RXIS = 3;

	return 0;
}


int init_antenna_uart(void) {
	int err = 0;

	UART2_RX_PS();
	UART2_TX_PS();

	/* init UART2 bus with 115200 baudrate */
	U2MODEbits.ON = 1;
	U2STAbits.URXEN = 1;
	U2STAbits.UTXEN = 1;
	U2BRG = 82;

	/*
	 * UART2 interrupts:
	 * enable RX and TX interrupt
	 * priority = 1, sub-priority = 3
	 */
	IFS4bits.U2RXIF = 0;
	IFS4bits.U2TXIF = 0;
	IEC4bits.U2RXIE = 1;
	IEC4bits.U2TXIE = 0;
	IPC36bits.U2RXIP = 1;
	IPC36bits.U2TXIP = 1;
	IPC36bits.U2RXIS = 3;
	IPC36bits.U2TXIS = 3;

	return err;
}


int init_avionics_uart(void) {
	UART3_RX_PS();
	UART3_TX_PS();

	/* init UART3 bus with 115200 baudrate */
	U3MODEbits.ON = 1;
	U3STAbits.URXEN = 1;
	U3STAbits.UTXEN = 1;
	U3BRG = 82;

	return 0;
}


void __ISR_AT_VECTOR(_UART1_RX_VECTOR, IPL1SRS) _uart1_rx_isr_h(void) {
	/* clear UART interrupt flag */
	IFS3bits.U1RXIF = 0;
}


void __ISR_AT_VECTOR(_UART2_RX_VECTOR, IPL1SRS) _uart2_rx_isr_h(void) {
	/* clear UART interrupt flag */
	IFS4bits.U2RXIF = 0;
}

//
//void __ISR_AT_VECTOR(_UART2_TX_VECTOR, IPL1SRS) _uart2_tx_isr_h(void) {
//	/* clear UART interrupt flag */
//	IFS4bits.U2TXIF = 0;
//}


