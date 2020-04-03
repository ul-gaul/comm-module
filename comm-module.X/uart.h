/*
 * Library for uart initialisations
 */


#ifndef _UART_H_
#define _UART_H_


/* includes */
#include "xc.h"


/*
 * Function-like macros to init pin-select registers
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


/* public functions */
int init_motor_control_uart(void);
int init_antenna_uart(void);
int init_avionics_uart(void);

#endif /* _UART_H_ */
