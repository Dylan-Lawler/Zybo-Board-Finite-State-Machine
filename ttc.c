/*
 * Dylan Lawler, Winter 2023
 */

#include "gic.h"
#include "ttc.h"

static XTtcPs ttc;					/* the device instance */
static XTtcPs_Config *ttc_config;	/* its configuration */

static void (*saved_ttc_callback)(void);

/*
 * ttc_handler -- invoked whenever the timer overflows
 */
static void ttc_handler(void *devicep) {
	u32 status;

	/* coerce the generic pointer into a ttc device ptr */
	XTtcPs *ttcp = (XTtcPs*)devicep;

	/* invoke the saved callback function */
	saved_ttc_callback();

	/* get the interrupt status */
	status=XTtcPs_GetInterruptStatus(ttcp);
	/* clear the interrupt */
	XTtcPs_ClearInterruptStatus(ttcp,status);
}

/*
 * ttc_init -- initialize the ttc for period (ms)
 *
 * freq -- timer frequency
 * ttc_callback -- function to call every time it goes off
 *
 */
void ttc_init(u32 freq, void (*ttc_callback)(void)) {
	u8 prescaler;
	XInterval interval;

	/* the calback function */
	saved_ttc_callback=ttc_callback;

	/* lookup and initialize ttc timer 0 */
	ttc_config = XTtcPs_LookupConfig(XPAR_XTTCPS_0_DEVICE_ID);
	if(XTtcPs_CfgInitialize(&ttc,ttc_config,ttc_config->BaseAddress) != XST_SUCCESS) {
		printf("[ERROR: can't initialize ttc]\n");
		return;
	}
	/* make sure its not generating interrupts */
	XTtcPs_DisableInterrupts(&ttc,XTTCPS_IXR_INTERVAL_MASK);
	/* connect it into the gic */
	if(gic_connect(XPAR_XTTCPS_0_INTR,(Xil_InterruptHandler)ttc_handler,(void*)&ttc) != XST_SUCCESS) {
		printf("[ERROR: can't connect ttc to gic]\n");
		return;
	}
	/* set the prescaler and interval */
	XTtcPs_CalcIntervalFromFreq(&ttc,freq,&interval,&prescaler);
	XTtcPs_SetPrescaler(&ttc,prescaler);
	XTtcPs_SetInterval(&ttc,interval);
	/* set interval mode */
	XTtcPs_SetOptions(&ttc,XTTCPS_OPTION_INTERVAL_MODE);
}

void ttc_start(void) {
	/* enable it to generate interrupts */
	XTtcPs_EnableInterrupts(&ttc,XTTCPS_IXR_INTERVAL_MASK);
	/* start it counting */
	XTtcPs_Start(&ttc);
}

void ttc_stop(void) {
	/* stop it generating interrupts */
	XTtcPs_DisableInterrupts(&ttc,XTTCPS_IXR_INTERVAL_MASK);
	/* stop it counting */
	XTtcPs_Stop(&ttc);
}

void ttc_close(void) {
	gic_disconnect(XPAR_XTTCPS_0_INTR);
}
