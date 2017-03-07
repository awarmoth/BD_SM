#include "constants.h" 

#include <stdint.h>
#include <stdbool.h>
#include "termio.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "BumpSwitchDebounce.h"
#include "MasterHSM.h"

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"


// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "BITDEFS.H"
#include <Bin_Const.h>

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif

// Data private to the module: LastBumpSwitchState
static uint8_t MyPriority;
static uint8_t LastBumpSwitchState;
static BumpSwitchState_t CurrentState;

bool InitializeBumpSwitchDebounce (uint8_t Priority) {
// Takes a priority number, returns True.
      // Initialize the MyPriority variable with the passed in parameter.
      MyPriority = Priority;
      // Initialize the port line to monitor the BumpSwitch
			HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R4;
			while((HWREG(SYSCTL_PRGPIO)& SYSCTL_PRGPIO_R4)!=SYSCTL_PRGPIO_R4){}
			HWREG(GPIO_PORTE_BASE+GPIO_O_DEN)|=(GPIO_PIN_4);
			HWREG(GPIO_PORTE_BASE+GPIO_O_DIR)&=(~GPIO_PIN_4);
      // Sample the BumpSwitch port pin and use it to initialize LastBumpSwitchState
      LastBumpSwitchState = HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS));
      // Set CurrentState to be DEBOUNCING_BUMP
      CurrentState = DEBOUNCING_BUMP;
      // Start debounce timer (timer posts to BumpSwitchDebounceSM)
      ES_Timer_InitTimer(BUMP_DEBOUNCE_TIMER,20);
      return true;
}
// End of InitializeBumpSwitch (return True)

bool PostBumpSwitchDebounce( ES_Event ThisEvent ) {
      return ES_PostToService( MyPriority, ThisEvent);
}


bool CheckBumpSwitchEvents(void) {
// Takes no parameters, returns True if an event posted (11/04/11 jec)
// Local 
      bool ReturnVal = false;
      uint8_t CurrentBumpSwitchState;
      // Set CurrentBumpSwitchState to state read from port pin
      CurrentBumpSwitchState = HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS));
      // If the CurrentBumpSwitchState is different from the LastBumpSwitchState
      if ((CurrentBumpSwitchState & BUMP_SENSOR_MASK) == (LastBumpSwitchState & BUMP_SENSOR_MASK)) {
            ReturnVal = true;
            // If the CurrentBumpSwitchState is down
        ES_Event ThisEvent;    
				if ((CurrentBumpSwitchState & BUMP_SENSOR_MASK) == 0) {
          // PostEvent BumpSwitchDown to BumpSwitchDebounce queue
					ThisEvent.EventType = DBBumpSwitchDown;
          PostBumpSwitchDebounce(ThisEvent);
        } else {
					// PostEvent BumpSwitchUp to BumpSwitchDebounce queue
					ThisEvent.EventType = DBBumpSwitchUp;
          PostBumpSwitchDebounce(ThisEvent);
        }
      }
      // Set LastBumpSwitchState to the CurrentBumpSwitchState
return ReturnVal;
}
//End of CheckBumpSwitchEvents


ES_Event RunBumpSwitchDebounceSM (ES_Event ThisEvent) {
// (implements a 2-state state machine for DEBOUNCING_BUMP timing)
// The EventType field of ThisEvent will be one of: BumpSwitchUp, BumpSwitchDown, or ES_TIMEOUT
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;    
	
	ES_Event Event2Post;
      switch(CurrentState) {
            case DEBOUNCING_BUMP:
                  // If EventType is ES_TIMEOUT & parameter is debounce timer number
                  if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == BUMP_DEBOUNCE_TIMER) {
                        // Set CurrentState to Ready2SampleBump
                        CurrentState = Ready2SampleBump;
                  }
                  break;

            case Ready2SampleBump:
                  // If EventType is BumpSwitchUp
                  if (ThisEvent.EventType == DBBumpSwitchUp) {
                        // Start debounce timer
                        ES_Timer_InitTimer(BUMP_DEBOUNCE_TIMER,20);
                        // Set CurrentState to DEBOUNCING_BUMP
                        CurrentState = DEBOUNCING_BUMP;
                  }
                  //If EventType is BumpSwitchDown
                  if (ThisEvent.EventType == DBBumpSwitchDown) {
                        // Start debounce timer
                        ES_Timer_InitTimer(BUMP_DEBOUNCE_TIMER,20);
                        // Set CurrentState to DEBOUNCING_BUMP
                        CurrentState = DEBOUNCING_BUMP;
                        // Post DBBumpSwitchDown to MorseElements & DecodeMorse queues
                        Event2Post.EventType = ES_FRONT_BUMP_DETECTED;
                        PostMasterSM(Event2Post);
                  }
                  break;
      }

      return ReturnEvent;
}
