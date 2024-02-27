/*
 * Dylan Lawler, Winter 2023
 */

#include "uart.h"

/* uart modes */
#define ECHO 1 				/* echo usart0 to usart1 and vice versa */
#define GETLINE 2			/* get lines from uart1 */
#define SENDRECV 3			/* send & recv msgs via uart 0 */

#define BUFFSIZE 256		/* reply buffer size */

static u8 reply[BUFFSIZE];  /* msg buffer for replies */
static u8 *rp;				/* a pointer into a reply */
static int remaining;		/* number of bytes left to receive */

static void (*saved_line_callback)(void*); 		/* function to call on getline */
static void (*saved_recv_callback)(void*,int); 	/* function to call on msg reply */

static int uartmode;

/* the uarts */
static XUartPs UartPs0,UartPs1;
static XUartPs_Config *Config0,*Config1;


static void Uart0_Handler(void *CallBackRef, u32 Event, unsigned int EventData) {
	XUartPs *UartInstPtr = (XUartPs *) CallBackRef;

	if(Event==XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(UartInstPtr, rp, 1); 		/* recv byte from wifi */
		if(uartmode==ECHO)
			XUartPs_Send(&UartPs1,rp, 1);	 	/* send to screen */
		else if(uartmode==GETLINE) {
			;									/* throw it away */
		}
		else if(uartmode==SENDRECV) {
			rp++;								/* one less to put away */
			remaining--;
			if(remaining==0)					/* all arrived? */
				saved_recv_callback((void*)reply,rp-((u8*)reply)); 	/* use the callback */
		}
	}
}


static void Uart1_Handler(void *CallBackRef, u32 Event, unsigned int EventData) {
	XUartPs *UartInstPtr = (XUartPs *) CallBackRef;

	if (Event == XUARTPS_EVENT_RECV_DATA) {
		XUartPs_Recv(UartInstPtr,rp, 1);
		if(uartmode==ECHO) {
			XUartPs_Send(&UartPs0,rp,1); 		/* send to wifi */
			if(*rp==(u8)'\r') {					/* return ? */
				*rp=(u8)'\n';					/* print line feed to screen */
				XUartPs_Send(UartInstPtr,rp,1);
			}
		}
		else if(uartmode==GETLINE) {
			XUartPs_Send(UartInstPtr,rp, 1);	/* echo to screen */
			if(*rp==(u8)'\r') {					/* found EOL */
				*rp=(u8)'\n';					/* print a return */
				XUartPs_Send(UartInstPtr,rp,1);
				*rp = (u8)'\0';					/* terminate the string */
				saved_line_callback((void*)reply);	/* use the callback */
			}
			else
				rp++;
		}
		else if(uartmode==SENDRECV) {			/* uart1 not used in sendrecv */
			;									/* drop the character */
		}
	}
}

void uart_init() {
	/* initialize uart0 */
	Config0 = XUartPs_LookupConfig(XPAR_PS7_UART_0_DEVICE_ID);
	if(XUartPs_CfgInitialize(&UartPs0, Config0, Config0->BaseAddress)!=XST_SUCCESS) {
		printf("[Error initializing uart0]\n");
	}
	XUartPs_SetInterruptMask(&UartPs0,XUARTPS_IXR_RXOVR);
	XUartPs_SetFifoThreshold(&UartPs0, 1);
	XUartPs_SetHandler(&UartPs0, (XUartPs_Handler)Uart0_Handler, &UartPs0);
	XUartPs_SetBaudRate(&UartPs0,9600);
	gic_connect(XPAR_XUARTPS_0_INTR,(Xil_ExceptionHandler)XUartPs_InterruptHandler,(void *) &UartPs0);
	/* initialize uart1 */
	Config1 = XUartPs_LookupConfig(XPAR_PS7_UART_1_DEVICE_ID);
	if(XUartPs_CfgInitialize(&UartPs1, Config1, Config1->BaseAddress)!=XST_SUCCESS) {
		printf("[Error initializing uart1]\n");
	}
	XUartPs_SetInterruptMask(&UartPs1,XUARTPS_IXR_RXOVR);
	XUartPs_SetFifoThreshold(&UartPs1, 1);
	XUartPs_SetHandler(&UartPs1, (XUartPs_Handler)Uart1_Handler, &UartPs1);
	gic_connect(XPAR_XUARTPS_1_INTR,(Xil_ExceptionHandler)XUartPs_InterruptHandler,(void *) &UartPs1);
}

void uart_echo(void) {
	uartmode = ECHO;
	rp=reply;
}

void uart_getline(void (*f)(void*)) {
	uartmode = GETLINE;
	rp=reply;
	saved_line_callback = f;
}

void uart_msg(u8 *msgp,int msglen,void (*f)(void*,int),int resplen) {
	uartmode = SENDRECV;
	saved_recv_callback = f;			/* set up for the reply */
	remaining = resplen;				/* set how many bytes we need */
	rp = reply;							/* point to the reply */
	XUartPs_Send(&UartPs0,msgp,msglen);	/* send the message */
}

void uart_close() {
	XUartPs_DisableUart(&UartPs0);
	gic_disconnect(XPAR_XUARTPS_0_INTR);
	XUartPs_DisableUart(&UartPs1);
	gic_disconnect(XPAR_XUARTPS_1_INTR);
}
