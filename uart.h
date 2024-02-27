/*
 * uart.h -- Configures uart0 and uart1:
 * Dylan Lawler, Winter 2023
 */


#pragma once
#include <stdio.h>
#include "xuartps.h"
#include "gic.h"

/* initialize the uarts */
void uart_init();

/*
 * uart_echo -- echos characters from uart1 to uart0 and vise versa
 */
void uart_echo(void);

/*
 * uart_getline -- invokes a callback function when a line is typed on the keyboard
 */
void uart_getline(void (*f)(void* line));

/*
 * uart_msg -- send a msg on uart0 and invoke a callback function when a response
 * of the appropriate length is recieved
 */
void uart_msg(u8 *msgp,int msglen,void (*f)(void* response,int recvdlen),int lenrequired);

/* close down the usarts */
void uart_close();
