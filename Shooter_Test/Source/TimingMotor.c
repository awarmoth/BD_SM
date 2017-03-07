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
#include "termio.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_nvic.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "constants.h"
#include "PWM10Tiva.h"

#include "ADMulti.h"
#include "PWM_Module.h"

#define clrScrn() 	printf("\x1b[2J")
#define goHome()	printf("\x1b[1,1H")
#define clrLine()	printf("\x1b[K")

#define BITS_PER_NIBBLE 4
#define TICKS_PER_S 40000000

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif


/*----------------------------- Module Defines ----------------------------*/
#define NUM_MOTOR 1
#define MAX_MOT_POS 3200
#define MOT_FREQ 25000
#define INCREMENT 60
#define MOTOR_PERIOD 		250
#define TIMING_CHANNEL 	0
#define TIME_MOT_GROUP 	0
#define VIB_MOT_ON			1
#define VIB_MOT_OFF			0
#define S_PER_MIN 60
#define LAUNCHER_CONTROLLER_TIME_US 10000

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif


#define LAUNCHER_PULSE_PER_REV 3


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static TimingMotorState_t CurrentState;
static void Init_Launcher_Controller(void);
static void Launcher_Encoder_Init(void);


// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static uint16_t PulseWidth;
static uint8_t forward = 1;
static uint16_t Launcher_RPS = 0;
static uint32_t Last_Launcher_Time = 0;
static uint8_t Launcher_Command = 0;
static uint8_t CommandVal = 30;
static uint16_t ServoUpPosition = 770;
static uint16_t ServoDownPosition = 1540;



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
  InitFlywheelPWM();
 	Init_Launcher_Controller();
 	Launcher_Encoder_Init();
	printf("started");
	
	
  // put us into the Initial PseudoState
  CurrentState = MotorTest;
	
	// Enable the pin for the vibration motor
	// Enable Port F
	//HWREG(SYSCTL_RCGCGPIO) |= GPIO_PIN_5; 
	
	// Make sure the peripheral clock has been set up
	//while((HWREG(SYSCTL_PRGPIO) & BIT5HI) != BIT5HI)
	

	// Make PF4 digital and output
	//HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (GPIO_PIN_4);
	//HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= (GPIO_PIN_4);
	printf("\rHardware Init\r\n");
	printf("started2");
  
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
						printf("MOVED TO SERVO UP POSITION: %d\r\n", ServoUpPosition);
					} 
					else if(ThisEvent.EventParam == '2')
					{
						SetServoPosition(ServoDownPosition);
						printf("MOVED TO SERVO DOWN POSITION: %d\r\n", ServoDownPosition);
					}
					else if(ThisEvent.EventParam == '3')
					{
						ServoUpPosition = ServoUpPosition + 10;
						if(ServoUpPosition > 3000)
						{
							ServoUpPosition = 3000;
							printf("SERVO IS AT 3000 LIMIT\r\n");
						}
						SetServoPosition(ServoUpPosition);
						printf("NEW SERVO UP POSITION = %d\r\n", ServoUpPosition);
					}
					else if(ThisEvent.EventParam == '4')
					{
						
						if(ServoUpPosition >= 10)
						{
							ServoUpPosition = ServoUpPosition - 10;
						}
						else
						{
							ServoUpPosition = 0;
							printf("SERVO IS AT 0 LIMIT\r\n");
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
							printf("SERVO IS AT 3000 LIMIT\r\n");
						}
						SetServoPosition(ServoDownPosition);
						printf("NEW SERVO DOWN POSITION = %d\r\n", ServoDownPosition);
					}
					else if (ThisEvent.EventParam == '6')
					{
						if(ServoDownPosition >= 10)
						{
							ServoDownPosition = ServoDownPosition - 10;
						}
						else
						{
							ServoDownPosition = 0;
							printf("SERVO IS AT 0 LIMIT\r\n");
						}
						SetServoPosition(ServoDownPosition);
						printf("NEW SERVO DOWN POSITION = %d\r\n", ServoDownPosition);
					}
					else if(ThisEvent.EventParam == '7')
					{
						SetLauncherCommand(0);
						printf("LAUNCHER TURNED OFF\r\n");
					}
					else if(ThisEvent.EventParam == '8')
					{
						SetLauncherCommand(CommandVal);
						printf("LAUNCHER TURNED ON\r\n");
					}
					else if(ThisEvent.EventParam == '9')
					{
						if(CommandVal > 133)
						{
							CommandVal = 138;
							SetLauncherCommand(CommandVal);
						}
						else
						{
							CommandVal = CommandVal + 5;
							SetLauncherCommand(CommandVal);
						}
						printf("LAUNCHER COMMAND = %d\r\n", Launcher_Command);
					}
					else if(ThisEvent.EventParam == '0')
					{
						if(CommandVal < 5)
						{
							CommandVal = 0;
							SetLauncherCommand(CommandVal);
						}
						else
						{
							CommandVal = CommandVal - 5;
							SetLauncherCommand(CommandVal);
						}
						printf("LAUNCHER COMMAND = %d\r\n", Launcher_Command);
					}						
				}//end ES_NEW_KEY block
				break;
			}//end case block
	}//end switch
	return ReturnEvent;
}

static void Launcher_Encoder_Init(void)
{
	//enable clock to timer
	HWREG(SYSCTL_RCGCWTIMER)|=SYSCTL_RCGCWTIMER_R1;
	//enable clock to port C
	HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R2;
	//wait for clock to connect
	while((HWREG(SYSCTL_PRWTIMER)&SYSCTL_PRWTIMER_R1)!=SYSCTL_PRWTIMER_R1) 
	{
	}
	//disable the Timer B
	HWREG(WTIMER1_BASE+TIMER_O_CTL)&=(~TIMER_CTL_TBEN);
	//set up 32 bit mode
	HWREG(WTIMER1_BASE+TIMER_O_CFG)=TIMER_CFG_16_BIT;
	//load the full value for timeout
	HWREG(WTIMER1_BASE+TIMER_O_TBILR)=0xffffffff;
	//set up timer B for capture mode, edge timer, periodic, and up counting
	HWREG(WTIMER1_BASE+TIMER_O_TBMR)=(HWREG(WTIMER1_BASE+TIMER_O_TBMR)&~TIMER_TBMR_TBAMS)|(TIMER_TBMR_TBCMR|TIMER_TBMR_TBCDIR|TIMER_TBMR_TBMR_CAP);
	//set event to rising edge
	HWREG(WTIMER1_BASE+TIMER_O_CTL)&=(~TIMER_CTL_TBEVENT_M);
	//set up the alternate function for Pin C7
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL)|=GPIO_PIN_7;
	//set up C4 alternate function as WTIMER0
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL)=(HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL)&0x0FFFFFFF)|(7<<(7*BITS_PER_NIBBLE));
	//digitally enable C7
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN)|=GPIO_PIN_7;
	//set C7 to input
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR)&=(~GPIO_PIN_7);
	//enable local capture interupt on the timer
	HWREG(WTIMER1_BASE+TIMER_O_IMR)|=TIMER_IMR_CBEIM;
	//enable timer interupt in the NVIC
	HWREG(NVIC_EN3)|=BIT1HI;
	//enable interupts globally
	__enable_irq();
	//set priority to 7
	HWREG(NVIC_PRI24)=(HWREG(NVIC_PRI24)&~NVIC_PRI24_INTB_M)|(0x7<<NVIC_PRI24_INTB_S);
	//enable the timer and add debugging stalls
	HWREG(WTIMER1_BASE+TIMER_O_CTL)|=(TIMER_CTL_TBSTALL|TIMER_CTL_TBEN);
}

void Launcher_Encoder_ISR(void)
{
	//clear interupt
	HWREG(WTIMER1_BASE+TIMER_O_ICR)=TIMER_ICR_CBECINT;
	//get interupt timer
	uint32_t Launcher_Time = HWREG(WTIMER1_BASE+TIMER_O_TBR);
	//calculate new RPM
	Launcher_RPS = ((TICKS_PER_S/LAUNCHER_PULSE_PER_REV)/(Launcher_Time-Last_Launcher_Time));
	//update time of last interupt
	Last_Launcher_Time=Launcher_Time;
	//Ignore RPM = 0 case
}

static void Init_Launcher_Controller(void)
{
	//enable clock to the timer (WTIMER3B)
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R3;
	while ((HWREG(SYSCTL_PRWTIMER)&SYSCTL_PRWTIMER_R3)!=SYSCTL_PRWTIMER_R3) {}
	//disable the timer
	HWREG(WTIMER3_BASE+TIMER_O_CTL)&=~(TIMER_CTL_TBEN);
	//set 32 bit wide mode
	HWREG(WTIMER3_BASE+TIMER_O_CFG)=TIMER_CFG_16_BIT;
	//set up periodic mode
	HWREG(WTIMER3_BASE+TIMER_O_TBMR)=(HWREG(WTIMER3_BASE+TIMER_O_TBMR)&~TIMER_TBMR_TBMR_M)|TIMER_TBMR_TBMR_PERIOD;
	//set timeout to 2 ms
	HWREG(WTIMER3_BASE+TIMER_O_TBILR)=(uint32_t)LAUNCHER_CONTROLLER_TIME_US*TICKS_PER_US;
	//enable local interupt
	HWREG(WTIMER3_BASE+TIMER_O_IMR)|=(TIMER_IMR_TBTOIM);
	//enable NVIC interupt
	HWREG(NVIC_EN3)|=(BIT5HI);
	//globally enable interrupts
	__enable_irq();
	//set priority to 6 (anything more important than the other ISRs)
	HWREG(NVIC_PRI25)=(HWREG(NVIC_PRI25)&~NVIC_PRI25_INTB_M)|(0x6<<NVIC_PRI25_INTB_S);
	//enable the timer
	HWREG(WTIMER3_BASE+TIMER_O_CTL)|=(TIMER_CTL_TBEN|TIMER_CTL_TBSTALL);
	SetLauncherCommand(0);
	
}

void Launcher_Controller_ISR (void)
{
	//clear interrupt
	HWREG(WTIMER3_BASE+TIMER_O_ICR)=TIMER_ICR_TBTOCINT;
	SetFlywheelDuty(Launcher_Command);
		//error is command minus RPM
	static float Kp = 20; //100
	static float Ki = 0.05;	//5
	static float Last_Launcher_Error = 0;
	static float Last_Launcher_Control = 0;
	float Launcher_Error = Launcher_Command - Launcher_RPS;
		//control is u[k]=(Kp+KiT/2)e[k]-(KiT/2-Kp)e[k-1]+u[k-1]
	float Launcher_Control = (Kp+Ki)*Launcher_Error + (Kp - Ki)*Last_Launcher_Error + Last_Launcher_Control;
		//if control is greater than nominal
	if (Launcher_Control > 100)
	{
			//control equals nominal
		Launcher_Control = 100;
			//update last control as nominal
		//Launcher_Error = Last_Launcher_Error;
	}
		//else if control is less than 0
	else if (Launcher_Control < 0)
	{
			//control is 0
		Launcher_Control = 0;
			//update last control as 0
		//Launcher_Error = Last_Launcher_Error;
	}
	//update previous errors and controls
	Last_Launcher_Error = Launcher_Error;
	Last_Launcher_Control = Launcher_Control;
	//write control to motors
	SetFlywheelDuty((uint8_t)Launcher_Control);
}

/*int main(void)
{
 	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
 			| SYSCTL_XTAL_16MHZ);
 	TERMIO_Init();
 	clrScrn();
 	printf("\r\nInitPWM\r\n");
  InitFlywheelPWM();
 	Init_Launcher_Controller();
 	Launcher_Encoder_Init();
 	while(1)
 	{
 	}
}
*/

void SetLauncherCommand(uint8_t InputCommand)
{
	Launcher_Command = InputCommand;
	if(InputCommand != 0)
	{
		HWREG(WTIMER3_BASE+TIMER_O_CTL)|=(TIMER_CTL_TBEN|TIMER_CTL_TBSTALL);
	}
	else
	{
		SetFlywheelDuty(0);
		HWREG(WTIMER3_BASE+TIMER_O_CTL)&=~(TIMER_CTL_TBEN);
	}
}
