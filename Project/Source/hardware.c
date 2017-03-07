/****************************************************************************
Module file for exectuing all hardware initialization

 ****************************************************************************/
 
 #include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

// Hardware definitions
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
#include "MasterHSM.h"

#define BITS_PER_NIBBLE 4
#define TICKS_PER_S 40000000

#ifndef ALL_BITS
#define ALL_BITS (0xff<<2)
#endif

// Frequency thresholds
#define LOWER_FREQ_THRESHOLD 1400
#define UPPER_FREQ_THRESHOLD 1500

#define ISR_TIMEOUT 100

static void Init_Controller(void);
static void AD_Init(void);
static void Init_Beacon_Receiver(void);
static void MagneticTimerInit(void);
static void OneShotTimerInit(void);
static void LoadingMotorInit(void);
static void Launcher_Encoder_Init(void);
static void Init_Launcher_Controller(void);
//static void LoadingMotorInit(void);
static void InitLEDs(void);
static void InitIR_Emitter(void);

static uint8_t Controller = CONTROLLER_OFF;
static uint8_t LastController = POSITION_CONTROLLER;
static uint8_t LeftCommand = 0;
static uint8_t RightCommand = 0;
static uint8_t RightResonanceSensor = FORWARD_RIGHT_RESONANCE_AD;
static uint8_t LeftResonanceSensor = FORWARD_LEFT_RESONANCE_AD;
static uint8_t TapeWatchFlag = 0;
static uint32_t RightResonanceHistory[TAPE_WATCH_WINDOW] = {0};
static uint32_t LeftResonanceHistory[TAPE_WATCH_WINDOW] = {0};
static bool ISR_Flag = false;
static uint32_t Last_Launcher_Time = 0;
static uint16_t Launcher_RPM = 0;
static uint16_t Launcher_Command = 0;

void InitializePins(void) {
	Init_Controller();
	AD_Init();
	Init_Beacon_Receiver();
	InitPWM();
	//InitFlywheelPWM();
	//SetFlywheelDuty(0);
	MagneticTimerInit();
	OneShotTimerInit();
	//Launcher_Encoder_Init();
	//Init_Launcher_Controller();
	//InitLEDs();
	//InitIR_Emitter();
}

static void Init_Controller(void)
{
	//enable clock to the timer (WTIMER2B)
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R2;
	while ((HWREG(SYSCTL_PRWTIMER)&SYSCTL_PRWTIMER_R2)!=SYSCTL_PRWTIMER_R2) {}
	//disable the timer
	HWREG(WTIMER2_BASE+TIMER_O_CTL)&=~(TIMER_CTL_TBEN);
	//set 32 bit wide mode
	HWREG(WTIMER2_BASE+TIMER_O_CFG)=TIMER_CFG_16_BIT;
	//set up periodic mode
	HWREG(WTIMER2_BASE+TIMER_O_TBMR)=(HWREG(WTIMER2_BASE+TIMER_O_TBMR)&~TIMER_TBMR_TBMR_M)|TIMER_TBMR_TBMR_PERIOD;
	//set timeout to 2 ms
	HWREG(WTIMER2_BASE+TIMER_O_TBILR)=(uint32_t)MOTOR_CONTROLLER_TIME_US*TICKS_PER_US;
	//enable local interupt
	HWREG(WTIMER2_BASE+TIMER_O_IMR)|=(TIMER_IMR_TBTOIM);
	//enable NVIC interupt
	HWREG(NVIC_EN3)|=(BIT3HI);
	//globally enable interrupts
	__enable_irq();
	//set priority to 6 (anything more important than the other ISRs)
	HWREG(NVIC_PRI24)=(HWREG(NVIC_PRI24)&~NVIC_PRI24_INTD_M)|(0x6<<NVIC_PRI24_INTD_S);
	//enable the timer
	HWREG(WTIMER2_BASE+TIMER_O_CTL)|=(TIMER_CTL_TBEN|TIMER_CTL_TBSTALL);
	
}


static void AD_Init(void)
{
	//set up AD pin
	HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R4;
	while((HWREG(SYSCTL_PRGPIO)& SYSCTL_PRGPIO_R4)!=SYSCTL_PRGPIO_R4){}
	HWREG(GPIO_PORTE_BASE+GPIO_O_DEN)|=(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	HWREG(GPIO_PORTE_BASE+GPIO_O_DIR)&=(~GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	ADC_MultiInit(4);
}

/****************************************************************************
 Function
    MagneticTimerInit

 Parameters
   None

 Returns
   None

 Description
   Function that sets up the encoder timer system
 Author
   Matthew Miller			1/19/17
****************************************************************************/
static void MagneticTimerInit(void)
{
	// Enable the clock to the timer
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R2;
	
	// Enable the clock to Port D
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R3;
	
	// Make sure the timer is disabled before configuring
	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	
	// Set up timer in 32 bit wide mode
	HWREG(WTIMER2_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	
	// Initialize the interval load register to max/default value
	HWREG(WTIMER2_BASE+TIMER_O_TAILR) = 0xffffffff;
	
	// Set up timer in capture mode
	// Set up timer for edge time
	// Set up timer for up-counting
	HWREG(WTIMER2_BASE+TIMER_O_TAMR) = (HWREG(WTIMER2_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAAMS) | (TIMER_TAMR_TACDIR | TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP);
	
	// Set the event to rising edge
	HWREG(WTIMER2_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEVENT_M;
	
	// Set up the alternate function for Port D0
	HWREG(GPIO_PORTD_BASE+GPIO_O_AFSEL) |= BIT0HI;
	
	// Map bit 0’s alternate function
	HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTD_BASE+GPIO_O_PCTL) & 0xfffffff0) + 7;
	
	// Enable pin on port D for digital I/O
	HWREG(GPIO_PORTD_BASE+GPIO_O_DEN) |= BIT0HI;
	
	// Make pin 0 on port D into input
	HWREG(GPIO_PORTD_BASE+GPIO_O_DIR) &= BIT0LO;
	
	// Enable local capture interrupt
	HWREG(WTIMER2_BASE+TIMER_O_IMR) |= TIMER_IMR_CAEIM;
	
	// Enable timer A in NVIC
	HWREG(NVIC_EN3) |= BIT2HI;
	
	// Enable interrupts globally
	__enable_irq();
	
	// Enable timer and enable to stall when stopped by debugger
	HWREG(WTIMER2_BASE+TIMER_O_CTL) |= (TIMER_CTL_TASTALL);
}

/****************************************************************************
 Function
    LoadingMotorInit

 Parameters
   None

 Returns
   None

 Description
   Function that sets up the encoder timer system
 Author
   Matthew Miller			1/19/17
****************************************************************************/
//static void LoadingMotorInit(void)
//{
//	// initialize PWM port
//	PWM_TIVA_Init(NUM_MOTOR);
//	
//	// initialize period of the timing motor
//	PWM_TIVA_SetPeriod(MOT_FREQ, TIME_MOT_GROUP);
//}

/****************************************************************************
 Function
    OneShotTimerInit

 Parameters
   None

 Returns
   None

 Description
   Function that sets up the one shot timer system to determine if the robot has passed a station
 Author
   Matthew Miller			1/19/17
****************************************************************************/
static void OneShotTimerInit(void)
{
	// Enable the clock to the timer
	HWREG(SYSCTL_RCGCWTIMER) |= SYSCTL_RCGCWTIMER_R3;
	
	// Make sure the clock has gotten going
	while((HWREG(SYSCTL_PRWTIMER) & SYSCTL_PRWTIMER_R3) != SYSCTL_PRWTIMER_R3){
	}
	
	// Make sure the timer is disabled before configuring
	HWREG(WTIMER3_BASE+TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
	
	// Set up timer in 32 bit wide mode
	HWREG(WTIMER3_BASE+TIMER_O_CFG) = TIMER_CFG_16_BIT;
	
	// Set up timer in one shot mode
	HWREG(WTIMER3_BASE+TIMER_O_TAMR) = (HWREG(WTIMER3_BASE+TIMER_O_TAMR) & ~TIMER_TAMR_TAMR_M) | TIMER_TAMR_TAMR_1_SHOT;
	
	// Set the timeout
	HWREG(WTIMER3_BASE+TIMER_O_TAILR) = ONE_SHOT_TIMEOUT;
	
	// Enable a local timeout interrupt
	HWREG(WTIMER3_BASE+TIMER_O_IMR) |= TIMER_IMR_TATOIM;
	
	// Enable timer A in NVIC
	HWREG(NVIC_EN3) |= BIT4HI;
	
	// Enable interrupts globally
	__enable_irq();
	
	// Enable timer and enable to stall when stopped by debugger
	HWREG(WTIMER3_BASE+TIMER_O_CTL) |= TIMER_CTL_TASTALL;
}


void Motor_Controller_ISR(void)
{
	//clear interrupt
	HWREG(WTIMER2_BASE+TIMER_O_ICR)=TIMER_ICR_TBTOCINT;
	static float LastError_POS = 0;
	static float LastControl_POS = 0;
	static float Kp_POS = 0.05;
	static float Kd_POS = 0.005;
	int8_t LeftControl = 0;
	int8_t RightControl = 0;
	
	//write requested commands
	//if desired control is no controller
	if (Controller == CONTROLLER_OFF)
	{
		HWREG(WTIMER2_BASE+TIMER_O_CTL)&=~(TIMER_CTL_TBEN);
		//if the last controller was not off
		if (LastController != CONTROLLER_OFF)
		{
			//stop motors
			SetDutyA(0);
			SetDutyB(0);
		}
	}
	
	//else if desired control is velocity 
	else if (Controller == VELOCITY_CONTROLLER)
	{
		static uint8_t TapeCounter = 0;
		// if the last controller was not velocity
		if (LastController != VELOCITY_CONTROLLER)
		{
			// clear the counter
			TapeCounter = 0;
		}
		//write the command values directly (open loop)
		SetDutyA(RightCommand);
		SetDutyB(LeftCommand);
		// if there is a flag for watching the tape
		if (TapeWatchFlag == 1)
		{
			// shift the resonance sensor history
			for (uint8_t i = (TAPE_WATCH_WINDOW - 1); i > 0 ; i--)
			{
				RightResonanceHistory[i] = RightResonanceHistory[i-1];
				LeftResonanceHistory[i] = LeftResonanceHistory[i-1];
			}
			// read the resonance sensors and store history
			uint32_t TapeVals[4];
			ADC_MultiRead(TapeVals);
			RightResonanceHistory[0] = TapeVals[RightResonanceSensor];
			LeftResonanceHistory[0] = TapeVals[LeftResonanceSensor];
			// if the counter is less than the averaging window
			if (TapeCounter < TAPE_WATCH_WINDOW)
			{
				// increment the counter
				TapeCounter++;
			}
			// else
			else
			{
				uint32_t NewRightAverage = 0;
				uint32_t OldRightAverage = 0;
				uint32_t NewLeftAverage = 0;
				uint32_t OldLeftAverage = 0;
				// calculate the average of the latest and oldest periods
				for (uint8_t i = 0; i < (uint8_t)(TAPE_WATCH_WINDOW/2);i++)
				{
					NewRightAverage += RightResonanceHistory[i];
					OldRightAverage += RightResonanceHistory[i+(uint8_t)(TAPE_WATCH_WINDOW/2)];
					NewLeftAverage += LeftResonanceHistory[i];
					OldLeftAverage += LeftResonanceHistory[i+(uint8_t)(TAPE_WATCH_WINDOW/2)];
				}
				
				NewRightAverage = NewRightAverage/(TAPE_WATCH_WINDOW/2);
				OldRightAverage = OldRightAverage/(TAPE_WATCH_WINDOW/2);
				NewLeftAverage = NewLeftAverage/(TAPE_WATCH_WINDOW/2);
				OldLeftAverage = OldLeftAverage/(TAPE_WATCH_WINDOW/2);
				// if the average has decreased by at least the threshold
				if ((NewRightAverage < (OldRightAverage - TAPE_THRESHOLD)) || (NewLeftAverage < (OldLeftAverage - TAPE_THRESHOLD)))
				{
					// clear the flag
					TapeWatchFlag = 0;
					// clear the counter
					TapeCounter = 0;
					// post a tape detected event
					ES_Event ThisEvent;
					ThisEvent.EventType = ES_TAPE_DETECTED;
					if (TAPE_TEST) printf("Tape Detected by ISR");
					PostMasterSM(ThisEvent);
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// else if desired control strategy is velocity control
		//error is command minus RPM
		//control is u[k]=(Kp+KiT/2)e[k]-(KiT/2-Kp)e[k-1]+u[k-1]
		//update previous errors and controls
		//if control is greater than nominal
			//control equals nominal
			//update last control as nominal
	//write control to motors
	//else if desired control strategy is position control
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	else if (Controller == POSITION_CONTROLLER)
	{
		//read sensor values
		uint32_t MotorVals[4];
		ADC_MultiRead(MotorVals);
		float RightVal = MotorVals[RightResonanceSensor];
		float LeftVal = MotorVals[LeftResonanceSensor];
		//error is the difference between the command and (Left - Right) (error is positive for rightward drift)
		float Error = COMMAND_DIFF - (LeftVal - RightVal);
		//control is u[k]=(Kp+2Kd/T)*e[k]+(Kp-2Kd/T)e[k-1]-u[k-2]
		float Control = (Kp_POS + Kd_POS)*Error + (Kp_POS - Kd_POS)*LastError_POS-LastControl_POS;
		
		//if last controller was not position controller
		if (LastController != POSITION_CONTROLLER)
		{
			//clear all error/command history
			LastError_POS = 0;
			LastControl_POS = 0;
		}
		
		//if control is positive, want L-R to increase: slow L
		if (Control > 0)
		{
			//left control is the nominal - control
			LeftControl = LEFT_MAX_DUTY - Control;
			//right control is the nominal
			RightControl = RIGHT_MAX_DUTY;
			//if the left control is < 0
			if (LeftControl<0)
			{
				//left control is 0
				LeftControl = 0;
				//update control as nominal value
				Control = LEFT_MAX_DUTY;
			}
		}
		//else if control is negative, want L-R to decrease: slow R
		else if (Control < 0)
		{
			//left control is the nominal
			LeftControl = LEFT_MAX_DUTY;
			//right control is the nominal + control
			RightControl = RIGHT_MAX_DUTY + Control;
			//if the right control is < 0
			if (RightControl < 0)
			{
				//right control is 0
				RightControl = 0;
				//update last control as the nominal*-1
				Control = -1*RIGHT_MAX_DUTY;
			}
		}
		//update previous errors and controls
		LastControl_POS = Control;
		LastError_POS = Error;
		//write control values (A is Right, B is Left)
		SetDutyA((uint8_t)RightControl);
		SetDutyB((uint8_t)LeftControl);
	}
	
	//update controller history
	LastController = Controller;
}

void SetMotorController(uint8_t control){
	//Enable interrupts
	HWREG(WTIMER2_BASE+TIMER_O_CTL) |= TIMER_CTL_TBEN;
	// if rotation is set
	// adjust motor commands
	// adjust motor directions
	// controller is velocity
	if (control == ROTATE_CCW)
	{
		if (SM_TEST) printf("set motor CCW");
		SetDirectionA(RIGHT_CCW_DIR);
		SetDirectionB(LEFT_CCW_DIR);
		RightCommand = RIGHT_CCW_COMMAND;
		LeftCommand = LEFT_CCW_COMMAND;
		Controller = VELOCITY_CONTROLLER;
	}
	else if (control == ROTATE_CW)
	{
		SetDirectionA(RIGHT_CW_DIR);
		SetDirectionB(LEFT_CW_DIR);
		RightCommand = RIGHT_CW_COMMAND;
		LeftCommand = LEFT_CW_COMMAND;
		Controller = VELOCITY_CONTROLLER;
	}
	// else if driving on tape is set
	// direction is forward
	// adjust selected ResonanceSensors
	// controller is position
	else if (control == DRIVE_ON_TAPE_FORWARD)
	{
		SetDirectionA(FORWARD_DIR);
		SetDirectionB(FORWARD_DIR);
		RightResonanceSensor = FORWARD_RIGHT_RESONANCE_AD;
		LeftResonanceSensor = FORWARD_LEFT_RESONANCE_AD;
		Controller = POSITION_CONTROLLER;
	}
	else if (control == DRIVE_ON_TAPE_REVERSE)
	{
		SetDirectionA(REVERSE_DIR);
		SetDirectionB(REVERSE_DIR);
		RightResonanceSensor = REVERSE_RIGHT_RESONANCE_AD;
		LeftResonanceSensor = REVERSE_LEFT_RESONANCE_AD;
		Controller = POSITION_CONTROLLER;
	}
	else if (control == STOP_DRIVING)
	{
		Controller = CONTROLLER_OFF;
	}
}

void FindTape(void)
{
	// Set a flag to watch for tape crossings
	TapeWatchFlag = 1;
	// Initialize the Resonance sensor history
	uint32_t TapeVals[4];
	ADC_MultiRead(TapeVals);
	uint32_t RightResonanceVal = TapeVals[RightResonanceSensor];
	uint32_t LeftResonanceVal = TapeVals[LeftResonanceSensor];
	for (uint8_t i = 0; i <= TAPE_WATCH_WINDOW;i++)
	{
		RightResonanceHistory[i] = RightResonanceVal;
		LeftResonanceHistory[i] = LeftResonanceVal;
	}
}


static void Init_Beacon_Receiver(void)
{
	//enable clock to timer
	HWREG(SYSCTL_RCGCWTIMER)|=SYSCTL_RCGCWTIMER_R0;
	//enable clock to port C
	HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R2;	
	//wait for clock to connect
	while((HWREG(SYSCTL_PRWTIMER)&SYSCTL_PRWTIMER_R0)!=SYSCTL_PRWTIMER_R0) 
	{
	}
	//disable the Timer B
	HWREG(WTIMER0_BASE+TIMER_O_CTL)&=(~TIMER_CTL_TBEN);
	//enable 32 bit mode
	HWREG(WTIMER0_BASE+TIMER_O_CFG)=TIMER_CFG_16_BIT;
	//load the full value for timeout
	HWREG(WTIMER0_BASE+TIMER_O_TBILR)=0xffffffff;
	//set up timer B for capture mode, edge timer, periodic, and up counting
	HWREG(WTIMER0_BASE+TIMER_O_TBMR)=(HWREG(WTIMER0_BASE+TIMER_O_TBMR)&~TIMER_TBMR_TBAMS)|(TIMER_TBMR_TBCMR|TIMER_TBMR_TBCDIR|TIMER_TBMR_TBMR_CAP);
	//set event to rising edge
	HWREG(WTIMER0_BASE+TIMER_O_CTL)&=(~TIMER_CTL_TBEVENT_M);
	//set up the alternate function for Pin C5
	HWREG(GPIO_PORTC_BASE+GPIO_O_AFSEL)|=GPIO_PIN_5;
	//set up C5 alternate function as WTIMER0
	HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL)=(HWREG(GPIO_PORTC_BASE+GPIO_O_PCTL)&0xFF0FFFFF)|(7<<(5*BITS_PER_NIBBLE));
	//digitally enable C5
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN)|=GPIO_PIN_5;
	//set C5 to input
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR)&=(~GPIO_PIN_5);
	//enable local capture interupt on the timer
	HWREG(WTIMER0_BASE+TIMER_O_IMR)|=TIMER_IMR_CBEIM;
	//enable timer interupt in the NVIC
	HWREG(NVIC_EN2)|=BIT31HI;
	//enable interupts globally
	__enable_irq();
	//set priority to 6
	HWREG(NVIC_PRI23)=(HWREG(NVIC_PRI23)&~NVIC_PRI23_INTD_M)|(0x6<<NVIC_PRI23_INTD_S);
	// add debugging stalls
	HWREG(WTIMER0_BASE+TIMER_O_CTL)|=(TIMER_CTL_TBSTALL);
	// NOTE: not enabling interrupts yet
}


void Beacon_Receiver_ISR(void)
{
	//clear the source of the interrupt
	HWREG(WTIMER0_BASE+TIMER_O_ICR)=TIMER_ICR_CBECINT;
	ISR_Flag = true;
	static uint32_t LastTime = 0;
	static uint8_t counter = 0;
	//capture the time at which the beacon was detected
	uint32_t Time = HWREG(WTIMER0_BASE+TIMER_O_TBR);
	//calculate the frequency
	uint32_t Frequency = TICKS_PER_S/(Time-LastTime);
	//if the frequency of detection matches the expected beacon frequency
	if ((Frequency>=LOWER_FREQ_THRESHOLD)&&(Frequency<=UPPER_FREQ_THRESHOLD)){
		counter++;
	}
	else counter /= 2;
	//printf("%i\r\n",Frequency);
	if (counter == 4) {
		//Disable Beacon Detection
		HWREG(WTIMER0_BASE+TIMER_O_CTL)&=(~TIMER_CTL_TBEN);
		//post a beacon detected event
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_GOAL_BEACON_DETECTED;
		PostMasterSM(ThisEvent);
		counter = 0;
	}
	//update the last time of detection
	LastTime = Time;
}

void InitLEDs(void) {
	HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R5;
	while((HWREG(SYSCTL_PRGPIO)& SYSCTL_PRGPIO_R5)!=SYSCTL_PRGPIO_R5){}
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN)|=(GPIO_PIN_2 | GPIO_PIN_3);
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR)|=(GPIO_PIN_2 | GPIO_PIN_3);
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA+ALL_BITS)) &= ~(BIT2HI | BIT3HI);
}

void InitIR_Emitter(void) {
	HWREG(SYSCTL_RCGCGPIO)|=SYSCTL_RCGCGPIO_R5;
	while((HWREG(SYSCTL_PRGPIO)& SYSCTL_PRGPIO_R5)!=SYSCTL_PRGPIO_R5){}
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN)|=(GPIO_PIN_4);
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR)|=(GPIO_PIN_4);
	//	Set the IR Emitter line LOW
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA+ALL_BITS)) &= ~BIT4HI;
}


bool getISRFlag(void) {
	return ISR_Flag;
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
	Launcher_RPM = TICKS_PER_S/LAUNCHER_PULSE_PER_REV*S_PER_MIN/(Launcher_Time-Last_Launcher_Time);
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
	
}

void Launcher_Controller_ISR (void)
{
	//clear interrupt
	HWREG(WTIMER3_BASE+TIMER_O_ICR)=TIMER_ICR_TBTOCINT;
		//error is command minus RPM
		//control is u[k]=(Kp+KiT/2)e[k]-(KiT/2-Kp)e[k-1]+u[k-1]
		//update previous errors and controls
		//if control is greater than nominal
			//control equals nominal
			//update last control as nominal
		//else if control is less than 005
			//control is 0
			//update last control as 0
	//write control to motors
}

void Set_Launcher_Command(uint16_t Requested_Command)
{
	Launcher_Command = Requested_Command;
}
