#include "main.h"


/* global variables */
CommandPacket cmdpkt;
unsigned char serial_buf[32];
unsigned int i;


int main(void) {
	unsigned int size;

	if (init_all()) {
		while(1);
	}

	/* (test) set attributes for command packet */
	cmdpkt.start_char = COMMAND_START;
	cmdpkt.function = 1;
	cmdpkt.arg = 1;
	cmdpkt.crc = 0;

//	size = serialize_command_packet(&cmdpkt, serial_buf);

	i = 0;
	/* main loop */
	for(;;) {
//		cmdpkt.function = (i % 2 == 0) ? 1 : 2;
//		cmdpkt.arg = i % 7;
//		serialize_command_packet(&cmdpkt, serial_buf);
//		motor_control_send(serial_buf, size);
//		if (U2STAbits.URXDA == 1) {
//			serial_buf[i] = U2RXREG;
//			if (++i >= 32) i = 0;
//		}
	}

	return 0;
}


int init_all(void) {
	int err = 0;
	

	err = init_motor_control_uart();
	if (err) goto exit;

	err = init_antenna_uart();
	if (err) goto exit;

	err = init_avionics_uart();
	if (err) goto exit;

	/* enable multi-vector interrupt */
	INTCONbits.MVEC = 1;
	/* assign shadow set 7 through 1 to priority level 7 through 1 */
	PRISS = 0x76543210;
	/* enable interrupts */
	__builtin_enable_interrupts();

exit:
	return err;
}


int init_motor_control_uart(void) {
	UART1_RX_PS();
	UART1_TX_PS();

	/* init UART1 bus with 115200 baudrate */
	U1MODEbits.ON = 1;
	U1STAbits.URXEN = 1;
	U1STAbits.UTXEN = 1;
	U1BRG = 82;

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
	 * enable RX interrupt
	 * priority = 5, sub-priority = 2
	 */
	IEC4bits.U2RXIE = 1;
	IPC36bits.U2RXIP = 1;
	IPC36bits.U2RXIS = 3;

	/* init DMA for UART2 RX on channel 0 */
	err = init_antenna_dma();
	
	sas_cmd_received = none;

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


int init_antenna_dma(void) {
	/* enable DMA controller */
	DMACONbits.ON = 1;
	
	/* CRC enabled, poly len = 16, background mode, CRC on channel 0 */
	DCRCXOR = CRC_POLY;
	DCRCDATA = CRC_SEED;
	DCRCCONbits.PLEN = CRC_LEN - 1;
	DCRCCONbits.CRCEN = 1;

	/* start IRQ is UART2 RX (146) , no pattern matching */
	DCH0ECONbits.CHSIRQ = 146;
	DCH0ECONbits.SIRQEN = 1;

	/* source physical address is UART2 RX */
	DCH0SSA = KVA_TO_PA((void *) &U2RXREG);
	/* destination physical address is ground control buffer */
	DCH0DSA = KVA_TO_PA((void *) sas_rx_buf);
	/* source size and source byte offset */
	DCH0SSIZ = 1;
	DCH0SPTR = 0;
	/* destination size and destination byte offset */
	DCH0DSIZ = SAS_RX_BUF_SIZE;
	DCH0DPTR = 0;
	/* 1 byte per UART transfer */
	DCH0CSIZ = 1;

	/* enable block complete and error interrupts */
	DCH0INTbits.CHBCIE = 1;
	DCH0INTbits.CHERIE = 1;

	/* channel 0 on, auto re-enable, highest priority, no chaining */
	DCH0CONbits.CHEN = 1;
	DCH0CONbits.CHAEN = 1;
	DCH0CONbits.CHPRI = 3;

	/* clear DMA0 interrupt flag */
	IFS4bits.DMA0IF = 0;
	/* enable DMA channel 0 interrupt with priority 5 and sub-priority 2*/
	IEC4bits.DMA0IE = 1;
	IPC33bits.DMA0IP = 5;
	IPC33bits.DMA0IS = 2;

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


void __ISR_AT_VECTOR(_DMA0_VECTOR, IPL5SRS) _dma_antenna_interrupt_h(void) {
	unsigned int err;

	/* check for DMA errors */
	err = DCH0INT;

	/* check if CRC is good */
	if (DCRCDATA == 0) {
		sas_cmd_received = crc_valid;
	} else {
		sas_cmd_received = crc_error;
	}
	
	/* seed the CRC */
	DCRCDATA = CRC_SEED;

	/* clear DMA0 interrupt bits */
	DCH0INT &= ~0x000000ff;
	IFS4bits.DMA0IF = 0;
}


void __ISR_AT_VECTOR(_UART2_RX_VECTOR, IPL1SRS) _uart2_rx_isr_h(void) {
	/* clear UART interrupt */
	IFS4bits.U2RXIF = 0;
}
