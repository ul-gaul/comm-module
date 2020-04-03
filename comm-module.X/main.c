#include "main.h"


int main(void) {

	if (init_all()) {
		while(1);
	}

	/* reset command handler */
	motor_cmd_h.state = idle;
	motor_cmd_h.ack = none;

	/* route CRC to channel 7 (data to ground control station) */
	route_dma_crc(0);

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

	err = init_ack_rx_dma();
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
	 * enable RX interrupt
	 * priority = 1, sub-priority = 3
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

	/* CRC enabled, poly len = 16, background mode, CRC on channel 0 */
	DCRCXOR = CRC_POLY;
	DCRCDATA = CRC_SEED;
	DCRCCONbits.PLEN = CRC_LEN - 1;
	DCRCCONbits.CRCEN = 1;

	return 0;
}


int init_sas_rx_dma(void) {
	/* start IRQ is UART2 RX, no pattern matching */
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
	/* start IRQ is UART1 RX, no pattern matching */
	DCH1ECONbits.CHSIRQ = _UART1_RX_VECTOR;
	DCH1ECONbits.SIRQEN = 1;

	/* source physical address is UART1 RX register */
	DCH1SSA = KVA_TO_PA((void *) U1RXREG);
	/* destination physical address is the ack buffer */
	DCH1DSA = KVA_TO_PA((void *) ack_tx_buf);
	/* source size and offset */
	DCH1SSIZ = 1;
	DCH1SPTR = 0;
	/* destination size and offset */
	DCH1DSIZ = ACK_TX_BUF_SIZE;
	DCH1DPTR = 0;
	/* 1 byte per UART transfer */
	DCH1CSIZ = 1;

	/* enable block complete and error interrupts */
	DCH1INTbits.CHBCIE = 1;
	DCH1INTbits.CHERIE = 1;

	/* channel 1 off, highest priority */
	DCH1CONbits.CHEN = 0;
	DCH1CONbits.CHPRI = 3;

	/* clear DMA1 interrupt flag */
	IFS4bits.DMA1IF = 0;
	/* disable DMA1 interrupt with priority 5 and sub-priority 2 */
	IEC4bits.DMA1IE = 0;
	IPC33bits.DMA1IP = 5;
	IPC33bits.DMA1IS = 2;

	return 0;
}


int init_ack_tx_dma(void) {
	/* source physical address is ack buffer */
	DCH2SSA = KVA_TO_PA((void *) ack_tx_buf);
	/* destination physical address is UART2 TX */
	DCH2DSA = KVA_TO_PA((void *) &U2TXREG);
	/* source size and source offset */
	DCH2SSIZ = ACK_TX_BUF_SIZE;
	DCH2SPTR = 0;
	/* destination size and destination offset */
	DCH2DSIZ = 1;
	DCH2DPTR = 0;
	/* 1 byte per UART transfer */
	DCH2CSIZ = 1;

	/* enable block complete and error interrupt */
	DCH2INTbits.CHBCIE = 1;
	/* TODO: maybe it need to be source done instead of block done */
//	DCH2INTbits.CHSDIE = 1;
	DCH2INTbits.CHERIE = 1;

	/* channel 2 on, auto re-enable, highest priority, no chaining */
	DCH0CONbits.CHEN = 1;
	DCH0CONbits.CHAEN = 1;
	DCH0CONbits.CHPRI = 3;

	/* clear DMA2 interrupt flag */
	IFS4bits.DMA2IF = 0;
	/* disable DMA2 interrupt with priority 5 and sub-priority 2 */
	IEC4bits.DMA2IE = 0;
	IPC34bits.DMA2IP = 5;
	IPC34bits.DMA2IS = 2;

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


int route_dma_crc(int channel) {
	int err = 0;

	switch (channel) {
	case 0:
		/* background mode */
		DCRCCONbits.CRCCH = 0;
		DCRCCONbits.CRCAPP = 0;
		break;
	case 1:
		/* background mode */
		DCRCCONbits.CRCCH = 1;
		DCRCCONbits.CRCAPP = 0;
		break;
	case 2:
		/* append mode */
		DCRCCONbits.CRCCH = 2;
		DCRCCONbits.CRCAPP = 1;
		break;
	case 7:
		/* append mode */
		DCRCCONbits.CRCCH = 7;
		DCRCCONbits.CRCAPP = 1;
		break;
	default:
		/* do nothing, other channels don't use CRC */
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
		/* route UART1 to DMA1 (ACK buffer) */
		route_motor_control_uart(MCU_ROUTE_ACK);
		break;
	case waiting:
		/* do nothing, wait for DMA1 interrupt to receive ACK from motor */
		break;
	case done:
		/* route UART1 to DMA5 (data buffer) */
		route_motor_control_uart(MCU_ROUTE_DATA);
		route_dma_crc(2);
		/* send ACK to ground control by forcing a DMA2 transfer */
		DCH2ECONbits.CFORCE = 1;
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


void __ISR_AT_VECTOR(_DMA1_VECTOR, IPL5SRS) _dma_motor_ack_isr_h(void) {
	/* set state to done */
	motor_cmd_h.state = done;

	/* clear DMA1 interrupt bits */
	DCH1INT &= ~0x000000ff;
	IFS4bits.DMA1IF = 0;
}


void __ISR_AT_VECTOR(_DMA2_VECTOR, IPL5SRS) _dma_ack_tx_isr_h(void) {
	/* ack is done sending, re-route CRC to channel 7 */
	/*
	 * TODO: change from 0 to 7 when channel 0 auto routes it 
	 * when is starts to receive a command
	 */
	route_dma_crc(0);

	/* clear DMA2 interrupt bits */
	DCH2INT &= ~0x000000ff;
	IFS4bits.DMA2IF = 0;
}


void __ISR_AT_VECTOR(_UART1_RX_VECTOR, IPL1SRS) _uart1_rx_isr_h(void) {
	/* clear UART interrupt flag */
	IFS3bits.U1RXIF = 0;
}


void __ISR_AT_VECTOR(_UART2_RX_VECTOR, IPL1SRS) _uart2_rx_isr_h(void) {
	/* clear UART interrupt flag */
	IFS4bits.U2RXIF = 0;
}


