/*
 * Main source file for the communication module of the rocket
 * 
 * Authors:
 *	  Simon Gaudy
 *	  Maxime Guillemette
 *	  Elliot Lafront
 * 
 *	Brief:
 *		Module to handle the communication between the rocket and the 
 *		ground station.
 */


#ifndef _MAIN_H_
#define _MAIN_H_

/* standard lib includes */
#include "config_bits.h"
#include "xc.h"
#include <sys/kmem.h>
#include <sys/attribs.h>

/* lib includes */
#include "rocket-packet/rocket_packet.h"


/*
 * pin out definitions:
 * 	MOTOR CONTROL	RX: D2, TX: D3
 * 	ANTENNA			RX: D4, TX: D5
 * 	AVIONICS		RX: B3, TX: B9
 */
#define UART1_RX_PS() (U1RXR = 0b0000)
#define UART1_TX_PS() (RPD3R = 0b0001)
#define UART2_RX_PS() (U2RXR = 0b0100)
#define UART2_TX_PS() (RPD5R = 0b0010)
#define UART3_RX_PS() (U3RXR = 0b1000)
#define UART3_TX_PS() (RPB9R = 0b0001)

/* see rocket packet documentation for details */
#define CRC_POLY 0x1021
#define CRC_LEN 16
#define CRC_SEED 0xffff


/* buffer for commands from ground control station */
#define SAS_RX_BUF_SIZE 32
char __attribute__((coherent)) sas_rx_buf[SAS_RX_BUF_SIZE];
unsigned int sas_rx_buf_index;
unsigned int test;


/* public functions */
int init_all(void);
int init_motor_control_uart(void);
int init_antenna_uart(void);
int init_avionics_uart(void);
int init_antenna_dma(void);
int motor_control_send(uint8_t* src, unsigned int size);
//void __ISR_AT_VECTOR(_DMA0_VECTOR, IPL5AUTO) _dma_antenna_interrupt_h(void);


#endif /* _MAIN_H_ */
