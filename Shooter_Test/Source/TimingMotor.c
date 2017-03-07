/****************************************************************************
 Module
   TimingMotor.c

 Description
   This is the service that controls the behavior of both the timing motor and
	 the vibration motor

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 11/12/16 02:03 mwm     created it for ME218A project
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TimingMotor.h"
#include "PWM10Tiva.h"
#include "PWM_Module.h"
#include "BITDEFS.H"


/*----------------------------- Module Defines ----------------------------*/
#define NUM_MOTOR 1
#define MIN_MOT_POS 1500
#define MAX_MOT_POS 3200
#define MOT_FREQ 25000
#define INCREMENT 60
#define MOTOR_PERIOD 		250
#define TIMING_CHANNEL 	0
#define TIME_MOT_GROUP 	0
#define VIB_MOT_ON			1
#define VIB_MOT_OFF			0

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static TimingMotorState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

static uint16_t ServoUpPosition = 300;
static uint16_t ServoDownPosition = 2000;



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTimingMotor

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     Matthew Miller, 11/12/16
****************************************************************************/
bool InitTimingMotor ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
	
	// initialize PWM port
	InitServoPWM();
	
	
  // put us into the Initial PseudoState
  CurrentState = MotorTest;
	
	// Enable the pin for the vibration motor
	// Enable Port F
	HWREG(SYSCTL_RCGCGPIO) |= GPIO_PIN_5; 
	
	// Make sure the peripheral clock has been set up
	while((HWREG(SYSCTL_PRGPIO) & BIT5HI) != BIT5HI)
	
		;
	// Make PF4 digital and output
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (GPIO_PIN_4);
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= (GPIO_PIN_4);
	printf("\rHardware Initialized\r\n");
  
	// post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostTimingMotor

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
    Matthew Miller, 11/12/16
****************************************************************************/
bool PostTimingMotor( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTimingMotorSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   Matthew Miller			11/12/16
****************************************************************************/
ES_Event RunTimingMotorSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch ( CurrentState )
  {
    case MotorTest :       // If current state is initial Psedudo State
		{
        if ( ThisEvent.EventType == ES_NEW_KEY )// only respond to ES_Init
        {
					if(ThisEvent.EventParam == '1')
					{
					// Put motor into initial position
						SetServoPosition(ServoUpPosition);
						printf("MOVED TO SERVO UP POSITION\r\n");
					} 
					else if(ThisEvent.EventParam == '2')
					{
						SetServoPosition(ServoDownPosition);
						printf("MOVED TO SERVO DOWN POSITION\r\n");
					}
					else if(ThisEvent.EventParam == '3')
					{
						ServoUpPosition = ServoUpPosition + 10;
						if(ServoUpPosition > 3000)
						{
							ServoUpPosition = 3000;
							printf("SERVO IS AT 3000 LIMIT");
						}
						SetServoPosition(ServoUpPosition);
						printf("NEW SERVO UP POSITION = %d\r\n", ServoUpPosition);
					}
					else if(ThisEvent.EventParam == '4')
					{
						
						if(ServoUpPosition >= 10)
						{
							ServoUpPosition = ServoUpPosition - 10;
							ServoUpPosition = 0;
							printf("SERVO IS AT 0 LIMIT");
						}
						SetServoPosition(ServoUpPosition);
						printf("NEW SERVO UP POSITION = %d\r\n", ServoUpPosition);
					}
					
					else if (ThisEvent.EventParam == '5')
					{
						ServoDownPosition = ServoDownPosition + 10;
						if(ServoDownPosition > 3000)
						{
							ServoDownPosition = 3000;
							printf("SERVO IS AT 3000 LIMIT");
						}
						SetServoPosition(ServoDownPosition);
						printf("NEW SERVO DOWN POSITION = %d\r\n", ServoDownPosition);
					}
					else if (ThisEvent.EventParam == '6')
					{
						if(ServoUpPosition >= 10)
						{
							ServoUpPosition = ServoUpPosition - 10;
							ServoUpPosition = 0;
							printf("SERVO IS AT 0 LIMIT");
						}
						SetServoPosition(ServoDownPosition);
						printf("NEW SERVO DOWN POSITION = %d\r\n", ServoDownPosition);
					}
			 
				}//end ES_NEW_KEY block
				break;
			}//end case block
	}//end switch
	return ReturnEvent;
}
