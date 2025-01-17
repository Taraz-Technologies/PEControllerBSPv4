/**
 ********************************************************************************
 * @file    	main_controller.c
 * @author  	Waqas Ehsan Butt
 * @date    	October 5, 2021
 * @copyright 	Taraz Technologies Pvt. Ltd.
 *
 * @brief  Main Controller for this Application
 ********************************************************************************
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_header.h"
#include "user_config.h"
#include "control_library.h"
#include "adc_config.h"
#include "main_controller.h"
#include "open_loop_vf_controller.h"
#include "pecontroller_digital_in.h"
/*******************************************************************************
 * Defines
 ******************************************************************************/

/*******************************************************************************
 * Enums
 ******************************************************************************/

/*******************************************************************************
 * Structures
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Inverter3Ph_ResetSignal(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern adc_measures_t adcVals;
openloopvf_config_t openLoopVfConfig1 = {0};
// Other PELab configurations don't support multiple inverter configurations
#if PECONTROLLER_CONFIG == PLB_6PH || PECONTROLLER_CONFIG == PLB_MMC
openloopvf_config_t openLoopVfConfig2 = {0};
#endif
static volatile bool recompute = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief Initiates PWM for both inverters, and enable Disable signals and configure the relays
 */
void MainControl_Init(void)
{
	// Configure digital IOs
	BSP_DigitalPins_Init();
	// Activate input port
	BSP_Din_SetPortGPIO();
	// latch zero to the output state till PWM signals are enabled
	BSP_Dout_SetPortAsGPIO();
	BSP_Dout_SetPortValue(0);

#if PECONTROLLER_CONFIG == PLB_6PH || PECONTROLLER_CONFIG == PLB_3PH
	openLoopVfConfig1.inverterConfig.s1PinNos[0] = 1;
	openLoopVfConfig1.inverterConfig.s1PinNos[1] = 3;
	openLoopVfConfig1.inverterConfig.s1PinNos[2] = 5;
	BSP_Dout_SetAsIOPin(15, GPIO_PIN_SET);
	BSP_Dout_SetAsIOPin(16, GPIO_PIN_SET);
#ifdef PELAB_VERSION
#if	PELAB_VERSION >= 4
	BSP_Dout_SetAsIOPin(13, GPIO_PIN_SET);
	BSP_Dout_SetAsIOPin(14, GPIO_PIN_SET);
#endif
#endif
#elif PECONTROLLER_CONFIG == PLB_MMC
	openLoopVfConfig1.inverterConfig.s1PinNos[0] = 1;
	openLoopVfConfig1.inverterConfig.s1PinNos[1] = 3;
	openLoopVfConfig1.inverterConfig.s1PinNos[2] = 5;
	openLoopVfConfig1.inverterConfig.s1PinDuplicate = 7;
#elif PECONTROLLER_CONFIG == PLB_TNPC
	openLoopVfConfig1.inverterConfig.s1PinNos[0] = 1;
	openLoopVfConfig1.inverterConfig.s1PinNos[1] = 5;
	openLoopVfConfig1.inverterConfig.s1PinNos[2] = 9;
	openLoopVfConfig1.inverterConfig.s1PinDuplicate = 13;
#endif
	OpenLoopVfControl_Init(&openLoopVfConfig1, Inverter3Ph_ResetSignal);

	// Other PELab configurations don't support multiple inverter configurations
#if PECONTROLLER_CONFIG == PLB_6PH
	openLoopVfConfig2.inverterConfig.s1PinNos[0] = 7;
	openLoopVfConfig2.inverterConfig.s1PinNos[1] = 9;
	openLoopVfConfig2.inverterConfig.s1PinNos[2] = 11;
	OpenLoopVfControl_Init(&openLoopVfConfig2, NULL);
#elif PECONTROLLER_CONFIG == PLB_MMC
	openLoopVfConfig2.inverterConfig.s1PinNos[0] = 9;
	openLoopVfConfig2.inverterConfig.s1PinNos[1] = 11;
	openLoopVfConfig2.inverterConfig.s1PinNos[2] = 13;
	openLoopVfConfig2.inverterConfig.s1PinDuplicate = 15;
	OpenLoopVfControl_Init(&openLoopVfConfig2, NULL);
#endif
}

/**
 * @brief Call this function to run both inverter PWMs
 */
void MainControl_Run(void)
{
	BSP_PWM_Start(0xffff, false);
}

/**
 * @brief Call this function to stop the inverters
 */
void MainControl_Stop(void)
{
	BSP_PWM_Stop(0xffff, false);
}

/**
 * @brief Used to signal the computation for new duty cycle
 */
static void Inverter3Ph_ResetSignal(void)
{
	recompute = true;
}

/**
 * @brief Call this function to process the control loop.
 * If the new computation request is available new duty cycle values are computed and applied to all inverter legs
 */
void MainControl_Loop(void)
{
	if(recompute)
	{
		OpenLoopVfControl_Loop(&openLoopVfConfig1);
		// Other PELab configurations don't support multiple inverter configurations
#if PECONTROLLER_CONFIG == PLB_6PH || PECONTROLLER_CONFIG == PLB_MMC
		OpenLoopVfControl_Loop(&openLoopVfConfig2);
#endif
		recompute = false;
	}
}

/* EOF */
