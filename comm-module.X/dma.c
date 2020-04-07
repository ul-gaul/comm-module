#include "dma.h"


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


int init_sas_rx_dma(char* dst, unsigned int dst_size) {
	/* start IRQ is UART2 RX, no pattern matching */
	DCH0ECONbits.CHSIRQ = _UART2_RX_VECTOR;
	DCH0ECONbits.SIRQEN = 1;

	/* source physical address is UART2 RX */
	DCH0SSA = KVA_TO_PA((void *) &U2RXREG);
	/* destination physical address is ground control buffer */
	DCH0DSA = KVA_TO_PA((void *) dst);
	/* source size and source byte offset */
	DCH0SSIZ = 1;
	DCH0SPTR = 0;
	/* destination size and destination byte offset */
	DCH0DSIZ = dst_size;
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


int init_ack_rx_dma(char* dst, unsigned int dst_size) {
	/* start IRQ is UART1 RX */
	DCH1ECONbits.CHSIRQ = _UART1_RX_VECTOR;
	DCH1ECONbits.SIRQEN = 1;

	/* source physical address is UART1 RX register */
	DCH1SSA = KVA_TO_PA((void *) U1RXREG);
	/* destination physical address is the ack buffer */
	DCH1DSA = KVA_TO_PA((void *) dst);
	/* source size and offset */
	DCH1SSIZ = 1;
	DCH1SPTR = 0;
	/* destination size and offset */
	DCH1DSIZ = dst_size;
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


int init_crccalc_dma(void) {
	/* no offset on source or destination */
	DCH7SPTR = 0;
	DCH7DPTR = 0;

	/* enable block complete and error interrupt */
	DCH7INTbits.CHBCIE = 1;
	DCH7INTbits.CHERIE = 1;

	/* channel 7 on, auto re-enable, low priority, no chaining */
	DCH7CONbits.CHEN = 1;
	DCH7CONbits.CHAEN = 1;
	DCH7CONbits.CHPRI = 3;

	/* clear DMA2 interrupt flag */
	IFS4bits.DMA7IF = 0;
	/* disable DMA2 interrupt with priority 5 and sub-priority 2 */
	IEC4bits.DMA7IE = 1;
	IPC35bits.DMA7IP = 5;
	IPC35bits.DMA7IS = 2;

	return 0;
}

int crccalc(char* src, unsigned int size, unsigned int* crc) {
	/* configure source and destination */
	DCH7SSA = KVA_TO_PA((void *) src);
	DCH7DSA = KVA_TO_PA((void *) src);
	DCH7SSIZ = size;
	DCH7DSIZ = size;
	DCH7CSIZ = size;
	
	/* enable crc, route to channel 7 and background mode */
	DCRCCONbits.CRCEN = 1;
	DCRCCONbits.CRCCH = 7;
	DCRCCONbits.CRCAPP = 0;
	
	/* seed the CRC and start the transfer */
	DCRCDATA = CRC_SEED;

	DCH7ECONbits.CFORCE = 1;
	while (DCH7CONbits.CHBUSY == 1);

	*crc = DCRCDATA;
	
	/* re-seed the CRC */
	DCRCDATA = CRC_SEED;

	return 0;
}


int init_motor_data_rx_dma(void) {

	return 0;
}


int init_avionics_data_rx_dma(void) {

	return 0;
}

