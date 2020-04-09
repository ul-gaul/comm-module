#include "main.h"


int main(void) {
	
	/* set avionics and motor data pointers to their addresses */
	avionics_data_buf = (char *) &sas_tx_buf[1];
	motor_data_buf = (char *) &sas_tx_buf[1 + AVIONICS_DATA_SIZE];

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

	err = init_sas_rx_dma(sas_rx_buf, SAS_RX_BUF_SIZE);
	if (err) goto exit;

	err = init_ack_rx_dma(motor_data_buf, MOTOR_DATA_SIZE);
//	err = init_ack_rx_dma(ack_tx_buf, ACK_TX_BUF_SIZE);
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


int route_motor_control_uart(int route) {
	int err = 0;

	/* wait for DMA to end transfer if applicable */
//	while (DCH1CONbits.CHBUSY == 1);

	DCH1CONbits.CHAEN = 0;
	DCH1CONbits.CHEN = 0;

	switch (route) {
	case MCU_ROUTE_ACK:
		/* set destination to ack buffer */
		DCH1DSA = KVA_TO_PA((void *) ack_tx_buf);
		DCH1DSIZ = ACK_PACKET_SIZE;
		DCH1CONbits.CHPRI = 3;
		break;
	case MCU_ROUTE_DATA:
		/* set destination to motor data buffer */
		DCH1DSA = KVA_TO_PA((void *) ack_tx_buf);
		DCH1DSIZ = MOTOR_DATA_SIZE;
		DCH1CONbits.CHPRI = 0;
		break;
	default:
		err = 1;
		break;
	}

	DCH1CONbits.CHEN = 1;
	DCH1CONbits.CHAEN = 1;

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
//	case 2:
//		/* append mode */
//		DCRCCONbits.CRCCH = 2;
//		/* TODO: it should work in append mode */
////		DCRCCONbits.CRCAPP = 1;
//		DCRCCONbits.CRCAPP = 0;
//		break;
//	case 7:
//		/* append mode */
//		DCRCCONbits.CRCCH = 7;
//		DCRCCONbits.CRCAPP = 1;
//		break;
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
		}
		/* don't send command if invalid CRC or command */
		if (motor_cmd_h.ack != ack) {
			motor_cmd_h.state = cmd_not_executed;
			break;
		}
		/* route UART1 to ACK buffer */
		route_motor_control_uart(MCU_ROUTE_ACK);
		/* send command to motor control */
		motor_control_send(sas_rx_buf, CMD_PACKET_SIZE);
		/* set new state to waiting */
		motor_cmd_h.state = waiting;
		break;
	case waiting:
		/* do nothing, wait for DMA1 interrupt to receive ACK from motor */
		break;
	case cmd_not_executed:
		/* form acknowledge packet and copy it to the transmit buffer */
		motor_cmd_h.ackpkt.start_short = COMMAND_START;
		motor_cmd_h.ackpkt.id = motor_cmd_h.cmd.id;
		motor_cmd_h.ackpkt.ack = motor_cmd_h.ack;
		motor_cmd_h.ackpkt.crc = 0;
		pack_ack_packet(&motor_cmd_h.ackpkt, (uint8_t *) ack_tx_buf);
		motor_cmd_h.state = done;
		break;
	case cmd_executed:
		/* ack packet is already in the ack_tx_buffer */
		motor_cmd_h.state = done;
		break;
	case done:
		route_motor_control_uart(MCU_ROUTE_DATA);
		sas_ack_send();
		/* reset state machine */
		motor_cmd_h.state = idle;
		break;
	default:
		break;
	}

	return 0;
}


int sas_ack_send(void) {
	unsigned int crc;

	crccalc(ack_tx_buf, ACK_PACKET_SIZE, &crc);

	ack_tx_buf[ACK_PACKET_SIZE - 1] = (crc & 0x00ff) >> 0;
	ack_tx_buf[ACK_PACKET_SIZE - 2] = (crc & 0xff00) >> 8;

	antenna_send(ack_tx_buf, ACK_PACKET_SIZE);

	return 0;
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
	/* TODO: add condition here to not set state to done when receiving motor data*/
	motor_cmd_h.state = cmd_executed;

	/* clear DMA1 interrupt bits */
	DCH1INT &= ~0x000000ff;
	IFS4bits.DMA1IF = 0;
}

