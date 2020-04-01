#include "main.h"


/* global variables */
unsigned int i;


int main(void) {

	if (init_all()) {
		while(1);
	}

	/* (test) set attributes for command packet */
	cmdpkt.start_short = COMMAND_START;
	cmdpkt.function = 1;
	cmdpkt.arg = 1;
	cmdpkt.crc = 0;

	i = 0;
	/* reset command handler */
	motor_cmd_h.state = idle;
	motor_cmd_h.ack = none;
	/* main loop */
	for(;;) {
		/* execute tasks */
		run_motor_cmd();
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

	err = init_sas_rx_dma();
	if (err) goto exit;

	err = init_interrupts();
	if (err) goto exit;

	err = enable_dma();
	if (err) goto exit;

exit:
	return err;
}


int init_interrupts(void) {
	/* enable multi-vector interrupt */
	INTCONbits.MVEC = 1;
	/* assign shadow set 7 through 1 to priority level 7 through 1 */
	PRISS = 0x76543210;
	/* enable interrupts */
	__builtin_enable_interrupts();

	return 0;
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
	 * priority = 1, sub-priority = 2
	 */
	IEC4bits.U2RXIE = 1;
	IPC36bits.U2RXIP = 1;
	IPC36bits.U2RXIS = 3;

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


int enable_dma(void) {
	/* enable DMA controller */
	DMACONbits.ON = 1;

	return 0;
}


int init_sas_rx_dma(void) {
	/* CRC enabled, poly len = 16, background mode, CRC on channel 0 */
	DCRCXOR = CRC_POLY;
	DCRCDATA = CRC_SEED;
	DCRCCONbits.PLEN = CRC_LEN - 1;
	DCRCCONbits.CRCEN = 1;

	/* start IRQ is UART2 RX , no pattern matching */
	DCH0ECONbits.CHSIRQ = _UART2_RX_VECTOR;
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

int init_sas_tx_dma(void) {

	return 0;
}


int init_ack_rx_dma(void) {

	return 0;
}


int init_ack_tx_dma(void) {

	return 0;
}


int init_motor_data_rx_dma(void) {

	return 0;
}


int init_avionics_data_rx_dma(void) {

	return 0;
}


int route_motor_control_uart(int route) {
	int err = 0;

	switch (route) {
	case MCU_ROUTE_ACK:
		/* disable channel 5 of DMA */
		DISABLE_DMA_CHANNEL(5);
		/* enable channel 1 of DMA */
		ENABLE_DMA_CHANNEL(1);
		break;
	case MCU_ROUTE_DATA:
		/* disable channel 1 of DMA */
		DISABLE_DMA_CHANNEL(1);
		/* enable channel 5 of DMA */
		ENABLE_DMA_CHANNEL(5);
		break;
	default:
		err = 1;
		break;
	}

	return err;
}


int run_motor_cmd(void) {

	switch (motor_cmd_h.state) {
	case idle:
		/* no command received, do nothing */
		break;
	case cmd_received:
		/* unpack command packet */
		unpack_command_packet(&motor_cmd_h.cmd, (uint8_t *) sas_rx_buf);
		/* check that values are valid */
		if ((motor_cmd_h.cmd.function > 2)
			|| (motor_cmd_h.cmd.arg > 5)) {
			motor_cmd_h.ack = nack;
			motor_cmd_h.state = done;
			break;
		}
		/* send command to motor control */
		motor_control_send(sas_rx_buf, sizeof(CommandPacket));
		/* set new state to waiting */
		motor_cmd_h.state = waiting;
		break;
	case waiting:
		/* send acknowledge packet with NACK */
		/* TODO: testing, switching to done right away */
		motor_cmd_h.state = done;
		break;
	case done:
		/* send ACK to ground control */
		/* reset state */
		motor_cmd_h.state = idle;
		break;
	default:
		break;
	}

	return 0;
}


int motor_control_send(char* src, unsigned int size) {
	unsigned int i;

	for (i = 0; i < size; ++i) {
		U1TXREG = src[i];
		/* wait for TX buffer to be empty */
		while (U1STAbits.TRMT == 0);
		while (U1STAbits.UTXBF == 1);
	}

	return i;
}


void __ISR_AT_VECTOR(_DMA0_VECTOR, IPL5SRS) _dma_antenna_isr_h(void) {
	/* check if CRC is good */
	motor_cmd_h.state = cmd_received;
	if (DCRCDATA == 0) {
		motor_cmd_h.ack = ack;
	} else {
		motor_cmd_h.ack = nack;
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
