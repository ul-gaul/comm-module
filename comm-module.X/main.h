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

/* lib includes */
#include "rocket-packet/rocket_packet.h"
#include "dma.h"
#include "uart.h"


/* buffer for commands from ground control station */
#define SAS_RX_BUF_SIZE CMD_PACKET_SIZE
char __attribute__((coherent)) sas_rx_buf[SAS_RX_BUF_SIZE];

/* buffer for acknowledge packets to ground station */
#define ACK_TX_BUF_SIZE ACK_PACKET_SIZE
char __attribute__((coherent)) ack_tx_buf[ACK_TX_BUF_SIZE];

/* buffer for sending rocket packets to the ground control station */
#define SAS_TX_BUF_SIZE ROCKET_PACKET_SIZE
char __attribute__((coherent)) sas_tx_buf[SAS_TX_BUF_SIZE];

/* global struct to manage motor commands */
struct {
    enum {
        idle, cmd_received, waiting, cmd_not_executed, cmd_executed, done
    } state;
	enum {none, ack=ACK, nack=NACK} ack;
    CommandPacket cmd;
    AckPacket ackpkt;
} motor_cmd_h;


/* public functions */
int init_all(void);
int init_interrupts(void);
int route_motor_control_uart(int route);
int route_dma_crc(int channel);
int run_motor_cmd(void);
int motor_control_send(char* src, unsigned int size);
int sas_ack_send(void);


#endif /* _MAIN_H_ */
