#ifndef TimingMotor_H
#define TimingMotor_H

/****************************************************************************
 Module
     TimingMotor.h
 Description
     header file to the Timing Motor module
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/12/16		    mwm		 created for ME218A project
*****************************************************************************/
#include <stdint.h>

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"

#include "BITDEFS.H"

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif

// typedefs for the states
// State definitions for use with the query function
typedef enum { MotorTest} TimingMotorState_t ;

// Public Function Prototypes

bool InitTimingMotor ( uint8_t Priority );
bool PostTimingMotor( ES_Event ThisEvent );
ES_Event RunTimingMotorSM( ES_Event ThisEvent );

#endif //TimingMotor_H
