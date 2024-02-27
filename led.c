/*
 * Dylan Lawler, Winter 2023
 */
#include "led.h"

#define CHANNEL1 1				/* channel 1 of the GPIO port */

#define ALLON 0xF
#define ALLOFF 0x0
#define NUMLEDS 4

#define LED4PIN 7 /* LED4 is connected to MIO7 (c.f. Zybo ref manual) */

/* An instance of an AXI GPIO port */
static XGpio axiport;

/* An instance of a PS GPIO port */
static XGpioPs psport;
static XGpioPs_Config *psconfig;

/*
 * Initialize the leds
 */
void led_init(void) {
	/* initialize device AXI_GPIO_0 */
	if(XGpio_Initialize(&axiport, XPAR_AXI_GPIO_0_DEVICE_ID)!=XST_SUCCESS) {
		printf("[ERROR: initializing leds]\n");
		return;
	}
	/* set all the pins to be output */
	XGpio_SetDataDirection(&axiport, CHANNEL1, 0x0);	/* set tristate buffer to output */

	/* initialize PS GPIO configuration */
	if((psconfig = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID))==NULL){
		printf("[ERROR: looking up leds]\n");
		return;
	}
	/* initialize the configuration */
	if(XGpioPs_CfgInitialize(&psport, psconfig, psconfig->BaseAddr)!=XST_SUCCESS){
		printf("[ERROR: configuring leds]\n");
		return;
	}
	/* set direction of PS GPIO led4 pin to be output */
	XGpioPs_SetDirectionPin(&psport,LED4PIN, 1);
	/* output enable the pin */
	XGpioPs_SetOutputEnablePin(&psport,LED4PIN, 1);
}

/*
 * Set an led on or off
 */
void led_set(u32 led, bool tostate) {
	u32 status,mask,ledbit;

	if(led<=3) {
		status = XGpio_DiscreteRead(&axiport,CHANNEL1); /* get led status */
		ledbit = 1 << led; /* create mask for turning it on */
		mask = ~ledbit;
		if(tostate)
			status |= ledbit; /* set the bit */
		else
			status &= mask;   /* clear the bit */
		XGpio_DiscreteWrite(&axiport, CHANNEL1, status);
	}
	else if(led==4) {
		if(tostate)
			XGpioPs_WritePin(&psport,LED4PIN,0x1);
		else
			XGpioPs_WritePin(&psport,LED4PIN,0x0);
	}
	else if(led==ALL) {
		if(tostate) {
			XGpio_DiscreteWrite(&axiport, CHANNEL1, ALLON);
			XGpioPs_WritePin(&psport,LED4PIN,0x1);
		}
		else {
			XGpio_DiscreteWrite(&axiport, CHANNEL1, ALLOFF);
			XGpioPs_WritePin(&psport,LED4PIN,0x0);
		}
	}
}

/*
 * Get the status of an led
 */
bool led_get(u32 led) {
	u32 ledbit,status;

	if(led<=3) {
		ledbit = 1 << led;
		/* read the data register */
		status = XGpio_DiscreteRead(&axiport,CHANNEL1);
		/* mask for the one we want */
		status = status & ledbit;
		/* test it */
		if(status>0)
			return LED_ON;
	}
	else if(led==4) {
		/* get the value */
		status = XGpioPs_ReadPin(&psport,LED4PIN);
		if(status>0)
			return LED_ON;
	}
	return LED_OFF;
}

void led_toggle(u32 led) {
	if(led<NUMLEDS) {
		if(led_get(led))
			led_set(led,LED_OFF);
		else
			led_set(led,LED_ON);
	}
}


