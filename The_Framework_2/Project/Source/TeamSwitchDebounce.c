#include "constants.h" 

#include <stdint.h>
#include <stdbool.h>
#include "termio.h"

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "TeamSwitchDebounce.h"
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

// Data private to the module: LastTeamSwitchState
static uint8_t MyPriority;
static uint8_t LastTeamSwitchState;
static TeamSwitchState_t CurrentState;

bool InitializeTeamSwitchDebounce (uint8_t Priority) {
// Takes a priority number, returns True.
      // Initialize the MyPriority variable with the passed in parameter.
      MyPriority = Priority;
      // Initialize the port line to monitor the TeamSwitch
			HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R4;
			while((HWREG(SYSCTL_PRGPIO)& SYSCTL_PRGPIO_R4)!=SYSCTL_PRGPIO_R4){}
			HWREG(GPIO_PORTE_BASE+GPIO_O_DEN)|=(GPIO_PIN_5);
			HWREG(GPIO_PORTE_BASE+GPIO_O_DIR)&=(~GPIO_PIN_5);
      // Sample the TeamSwitch port pin and use it to initialize LastTeamSwitchState
      LastTeamSwitchState = HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS));
      // Set CurrentState to be DEBOUNCING_TEAM
      CurrentState = DEBOUNCING_TEAM;
      // Start debounce timer (timer posts to TeamSwitchDebounceSM)
      ES_Timer_InitTimer(TEAM_DEBOUNCE_TIMER,20);
      return true;
}
// End of InitializeTeamSwitch (return True)

bool PostTeamSwitchDebounce( ES_Event ThisEvent ) {
      return ES_PostToService( MyPriority, ThisEvent);
}


bool CheckTeamSwitchEvents(void) {
// Takes no parameters, returns True if an event posted (11/04/11 jec)
// Local 
      bool ReturnVal = false;
      uint8_t CurrentTeamSwitchState;
      // Set CurrentTeamSwitchState to state read from port pin
      CurrentTeamSwitchState = HWREG(GPIO_PORTE_BASE+(GPIO_O_DATA+ALL_BITS));
      // If the CurrentTeamSwitchState is different from the LastTeamSwitchState
      if ((CurrentTeamSwitchState & TEAM_COLOR_MASK) != (LastTeamSwitchState & TEAM_COLOR_MASK)) {
            ReturnVal = true;
            // If the CurrentTeamSwitchState is down
        ES_Event ThisEvent;    
				if ((CurrentTeamSwitchState & TEAM_COLOR_MASK) == 0) {
          // PostEvent TeamSwitchDown to TeamSwitchDebounce queue
					ThisEvent.EventType = DBTeamSwitchDown;
          PostTeamSwitchDebounce(ThisEvent);
        } else {
					// PostEvent TeamSwitchUp to TeamSwitchDebounce queue
					ThisEvent.EventType = DBTeamSwitchUp;
          PostTeamSwitchDebounce(ThisEvent);
        }
      }
      // Set LastTeamSwitchState to the CurrentTeamSwitchState
			LastTeamSwitchState = CurrentTeamSwitchState;
return ReturnVal;
}
//End of CheckTeamSwitchEvents


ES_Event RunTeamSwitchDebounceSM (ES_Event ThisEvent) {
// (implements a 2-state state machine for DEBOUNCING_TEAM timing)
// The EventType field of ThisEvent will be one of: TeamSwitchUp, TeamSwitchDown, or ES_TIMEOUT
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT;    
	
	ES_Event Event2Post;
      switch(CurrentState) {
            case DEBOUNCING_TEAM:
                  // If EventType is ES_TIMEOUT & parameter is debounce timer number
                  if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == TEAM_DEBOUNCE_TIMER) {
                        // Set CurrentState to Ready2SampleTeam
                        CurrentState = Ready2SampleTeam;
                  }
                  break;

            case Ready2SampleTeam:
                  // If EventType is TeamSwitchUp
                  if (ThisEvent.EventType == DBTeamSwitchUp) {
                        // Start debounce timer
                        ES_Timer_InitTimer(TEAM_DEBOUNCE_TIMER,20);
                        // Set CurrentState to DEBOUNCING_TEAM
                        CurrentState = DEBOUNCING_TEAM;
                        Event2Post.EventType = ES_TEAM_SWITCH;
                        PostMasterSM(Event2Post);
                  }
                  //If EventType is TeamSwitchDown
                  if (ThisEvent.EventType == DBTeamSwitchDown) {
                        // Start debounce timer
                        ES_Timer_InitTimer(TEAM_DEBOUNCE_TIMER,20);
                        // Set CurrentState to DEBOUNCING_TEAM
                        CurrentState = DEBOUNCING_TEAM;
                        Event2Post.EventType = ES_TEAM_SWITCH;
                        PostMasterSM(Event2Post);
                  }
                  break;
      }

      return ReturnEvent;
}
