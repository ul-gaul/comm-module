/*
 * Library for DMA initialisations
 */


#ifndef _DMA_H_
#define _DMA_H_

/* includes */
#include "xc.h"
#include <sys/kmem.h>
#include <sys/attribs.h>


/* macros */
#define ENABLE_DMA_CHANNEL(c) \
	do { \
		DCH ## c ## CONbits.CHEN = 1; \
		IEC4bits.DMA ## c ## IE = 1; \
	} while (0)

#define DISABLE_DMA_CHANNEL(c) \
	do { \
		DCH ## c ## CONbits.CHEN = 0; \
		IEC4bits.DMA ## c ## IE = 0; \
	} while (0)

/* CRC definitions, see rocket packet documentation for details */
#define CRC_POLY 0x1021
#define CRC_LEN 16

/*
 * warning: the hardware CRC is using the indirect algorithm,
 * we need to convert the direct seed value (0xffff) to 0x84cf
 * to get the seed for the indirect algorithm
 */
#define CRC_SEED 0x84cf

/* routes the motor control UART can take */
#define MCU_ROUTE_ACK 0
#define MCU_ROUTE_DATA 1


/* public functions */

int enable_dma(void);
int init_sas_rx_dma(char* dst, unsigned int dst_size);
int init_sas_tx_dma(void);
int init_ack_rx_dma(char* dst, unsigned int dst_size);
int init_crccalc_dma(void);
int crccalc(char* src, unsigned int size, unsigned int* crc);
int init_avionics_data_rx_dma(void);
extern void __ISR_AT_VECTOR(_DMA0_VECTOR, IPL5SRS) _dma_antenna_isr_h(void);
extern void __ISR_AT_VECTOR(_DMA1_VECTOR, IPL5SRS) _dma_motor_ack_isr_h(void);
extern void __ISR_AT_VECTOR(_DMA7_VECTOR, IPL5SRS) _dma_crccalc_isr_h(void);


#endif /* _DMA_H_ */
