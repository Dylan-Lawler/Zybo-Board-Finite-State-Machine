/*
* Dylan Lawler, Winter 2023
*/

#include "adc.h"

static XAdcPs XAdcInst;
static XAdcPs_Config *ConfigPtr;
static XAdcPs *XAdcInstPtr;

/*
 * initialize the adc module
 */
void adc_init(void) {
	/* set a pointer to the instance */
	XAdcInstPtr = &XAdcInst;
	/* initialize the XAdc */
	if((ConfigPtr = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID))==NULL) {
		printf("[ERROR: XADC configuration failed\n");
		return;
	}
	/* configure it */
	if(XAdcPs_CfgInitialize(XAdcInstPtr, ConfigPtr, ConfigPtr->BaseAddress)!=XST_SUCCESS) {
		printf("[ERROR: XADC initialization failed\n");
		return;
	}
	/* Self Test the XADC/ADC device */
	if(XAdcPs_SelfTest(XAdcInstPtr)!=XST_SUCCESS) {
		printf("[ERROR: XADC self test failed\n");
		return;
	}
	/* disable sequencer */
	XAdcPs_SetSequencerMode(XAdcInstPtr,XADCPS_SEQ_MODE_SAFE);
	/* disable alarms */
	XAdcPs_SetAlarmEnables(XAdcInstPtr, 0x0);

	/* configure sequencer to sample all channels continuously */
	/*XAdcPs_SetSeqInputMode(XAdcInstPtr, XADCPS_SEQ_CH_TEMP | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_AUX14);*/

	/* enable channels */
	XAdcPs_SetSeqChEnables(XAdcInstPtr, XADCPS_SEQ_CH_TEMP | XADCPS_SEQ_CH_VCCINT | XADCPS_SEQ_CH_AUX14);
	/* re-enable sequencer */
	XAdcPs_SetSequencerMode(XAdcInstPtr, XADCPS_SEQ_MODE_CONTINPASS);
	/* adc is now running and gathering data */
}

/*
 * get the internal temperature
 */
float adc_get_temp(void) {
	u32 TempRawData;
	float TempData;
	TempRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_TEMP);
	TempData = XAdcPs_RawToTemperature(TempRawData);
	return TempData;
}

/*
 * get the internal vcc
 */
float adc_get_vccint(void) {
	u32 VccIntRawData;
	float VccIntData;
	VccIntRawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_VCCINT);
	VccIntData = XAdcPs_RawToVoltage(VccIntRawData);
	return VccIntData;
}

/*
 * get the potentiometer voltage -- note: value returned is a factor of 3 higher than actual
 */
float adc_get_pot(void) {
	u32 Aux14RawData;
	float Aux14Data;
	/* Part 2 - Read data from Aux14 Channel AKA the Potentiometer*/
	Aux14RawData = XAdcPs_GetAdcData(XAdcInstPtr, XADCPS_CH_AUX_MAX-1);
	Aux14Data = XAdcPs_RawToVoltage(Aux14RawData)/3.0;
	return Aux14Data;
}
