/*
 * Dylan Lawler, Winter 2023
 */

#include "servo.h"

#define PERIOD 1000000             /* s_axi_aclk = 50MHz -- 20ms period = 1x10^6 * 1/(50*10^-9) */

#define MINPOINT ((double)5.5)
#define MAXPOINT ((double)10.25)
#define MIDPOINT ((double)7.5)		/* nominal midpoint */

static XTmrCtr tmr;

/*
 * Inialise the servo
 */
void servo_init(void) {
	u32 options;

	if(XTmrCtr_Initialize(&tmr,XPAR_AXI_TIMER_0_DEVICE_ID) != XST_SUCCESS) {
		printf("\n[ERROR: unable to initialize axi timer]\n");
		return;
	}
	/* make sure its not running */
	XTmrCtr_Stop(&tmr,0);
	XTmrCtr_Stop(&tmr,1);
	/* set the timer options -- compare, enable, count down */
	options = XTC_EXT_COMPARE_OPTION | XTC_PWM_ENABLE_OPTION | XTC_DOWN_COUNT_OPTION;
	XTmrCtr_SetOptions(&tmr,0,options);
	XTmrCtr_SetOptions(&tmr,1,options);
	/* set the duty factor and start the timer */
	servo_set(MIDPOINT);
}


/*
 * Update the servo to duty cycle <dc>
 */
void servo_set(double dutycycle) {
	u32 thigh;

	/* set the duty cycle */
	if(dutycycle<MINPOINT) {
		dutycycle=MINPOINT;
		printf("\n[ERROR: minimum limit exceeded]\n");
	}
	if(dutycycle>MAXPOINT) {
		dutycycle=MAXPOINT;
		printf("\n[ERROR: maximum limit exceeded]\n");
	}
	thigh = (u32)(dutycycle*(PERIOD/100));
	XTmrCtr_Stop(&tmr,0);
	XTmrCtr_Stop(&tmr,1);
	/* set the period */
	XTmrCtr_SetResetValue(&tmr,0,PERIOD);
	XTmrCtr_SetResetValue(&tmr,1,thigh);
	XTmrCtr_Start(&tmr,0);
	XTmrCtr_Start(&tmr,1);
}



