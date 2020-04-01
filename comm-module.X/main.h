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


/* function-like macros */
#define ENABLE_DMA_CHANNEL(c) (DCH ## c ## CONbits.CHEN = 1)
#define DISABLE_DMA_CHANNEL(c) (DCH ## c ## CONbits.CHEN = 0)


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

/*
 * warning: the hardware CRC is using the indirect algorithm,
 * we need to convert the direct seed value (0xffff) to 0x84cf
 * to get the seed for the indirect algorithm
 */
#define CRC_SEED 0x84cf

/* routes the motor control UART can take */
#define MCU_ROUTE_ACK 0
#define MCU_ROUTE_DATA 1

/* buffer for commands from ground control station */
#define SAS_RX_BUF_SIZE sizeof(CommandPacket)
char __attribute__((coherent)) sas_rx_buf[SAS_RX_BUF_SIZE];

/* buffer for sending rocket packets to the ground control station */
#define SAS_TX_BUF_SIZE (sizeof(AckPacket) + sizeof(RocketPacket))
char __attribute__((coherent)) sas_tx_buf[SAS_TX_BUF_SIZE];

/* global struct to manage motor commands */
struct {
    enum {idle, cmd_received, waiting, done} state;
	enum {none, ack=1, nack=0xff} ack;
    CommandPacket cmd;
} motor_cmd_h;


/* public functions */
int init_all(void);
int init_interrupts(void);
int init_motor_control_uart(void);
int init_antenna_uart(void);
int init_avionics_uart(void);
int init_dma(void);
int init_sas_rx_dma(void);
int init_sas_tx_dma(void);
int init_ack_rx_dma(void);
int init_ack_tx_dma(void);
int init_motor_data_rx_dma(void);
int init_avionics_data_rx_dma(void);
int route_motor_control_uart(int route);
int run_motor_cmd(void);
int motor_control_send(char* src, unsigned int size);


#endif /* _MAIN_H_ */
