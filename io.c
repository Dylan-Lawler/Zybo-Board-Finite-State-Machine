/*
 * Dylan Lawler, Winter 2023
 */
#include "io.h"
#include "gic.h"

#define CHANNEL1 (0x1)		/* channel 1 of the GPIO port */
#define INPUT 0xFFFFFFFF	/* setting GPIO direction to ALL input */

/* convert a bit posiiton to an interger */
static u32 convert(u32 status) {
	switch(status) {
	case 1: return 0;
	case 2: return 1;
	case 4: return 2;
	case 8: return 3;
	default: return -1;
	}
}

/*
 * Button Handling
 */
static XGpio btnport;       /* the btn GPIO port */
static void (*saved_btn_callback)(u32 btn);
static bool pushed; /* true if a button is pushed */

static void btn_handler(void *devicep) {
	u32 status,btn;
	/* coerce the generic pointer into a gpio */
	XGpio *dev = (XGpio*)devicep;

	if(!pushed) {
		status = XGpio_DiscreteRead(dev,CHANNEL1); /* get led status */
		btn=convert(status);
		/* invoke the saved callback function */
		saved_btn_callback(btn);
		pushed=true;
	}
	else {
		pushed=false;
	}
	/* clear the interrupt */
	XGpio_InterruptClear(dev,XGPIO_IR_CH1_MASK);
}

/*
 * switch handling
 */
static XGpio swport;       					/* the switch GPIO port */
static void (*saved_sw_callback)(u32 sw); 	/* switch callback */
static u32 prev_sw_status;                   	/* status last time a sw changed */

/* find out which switch changed */
static u32 get_changed(u32 status) {
	u32 changed;
	changed = prev_sw_status ^ status; /* xor to find what changed */
	prev_sw_status=status; /* save the new value */
	return convert(changed);
}

static void sw_handler(void *devicep) {
	u32 status,sw;
	/* coerce the generic pointer into a gpio */
	XGpio *dev = (XGpio*)devicep;

	status = XGpio_DiscreteRead(dev,CHANNEL1); /* get status */
	sw=get_changed(status);
	/* invoke the saved callback function */
	saved_sw_callback(sw);
	/* clear the interrupt */
	XGpio_InterruptClear(dev,XGPIO_IR_CH1_MASK);
}

/*
 * Initialize a port
 */
static void init_port(XGpio *port, void (*handler)(void *dev), u32 devid, u32 intid) {
	/* initialize gpio port */
	 if(XGpio_Initialize(port, devid)!=XST_SUCCESS) {
	  	printf("[ERROR: cant init device]\n");
	  	return;
	 }
	 /* immediately disable interrupts to the processor */
	 XGpio_InterruptGlobalDisable(port);
	 /* diable channel interrups */
	 XGpio_InterruptDisable(port, XGPIO_IR_CH1_MASK);
	 /* set tri-state buffer to input */
	 XGpio_SetDataDirection(port, CHANNEL1, INPUT);
	 /* connect handler to the device */
	 if(gic_connect(intid,handler,port)!=XST_SUCCESS) {
		printf("[ERROR: cant connect handler]\n");
		return;
	 }
	 /* enable the channel interrupt */
	 XGpio_InterruptEnable(port, XGPIO_IR_CH1_MASK);
	 /* enable interrupts to the processor */
	 XGpio_InterruptGlobalEnable(port);

}


/*
 * The public interface
 */

/*
 * Initialize the button port
 */
void io_btn_init(void (*btn_callback)(u32 btn)) {
	/* save the callback */
	saved_btn_callback=btn_callback;
	/* initialize the port */
	init_port(&btnport, btn_handler, XPAR_AXI_GPIO_1_DEVICE_ID, XPAR_FABRIC_GPIO_1_VEC_ID);
	/* no buttons pressed yet */
	pushed = false;
}

/*
 * Close the button port
 */
void io_btn_close(void) {
	gic_disconnect(XPAR_FABRIC_GPIO_1_VEC_ID);
}


/*
 * Initialize the switch port
 */
void io_sw_init(void (*sw_callback)(u32 sw)) {
	/* save the callback */
	saved_sw_callback=sw_callback;
	/* initialize the port */
	init_port(&swport, sw_handler, XPAR_AXI_GPIO_2_DEVICE_ID, XPAR_FABRIC_GPIO_2_VEC_ID);
	/* set the previous switch status */
	prev_sw_status = XGpio_DiscreteRead(&swport,CHANNEL1);
}

/*
 * Close the switch port
 */
void io_sw_close(void) {
	gic_disconnect(XPAR_FABRIC_GPIO_2_VEC_ID);
}
