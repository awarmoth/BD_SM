//module level variables: MyPriority, CurrentState
//FiringState_t: Waiting, MovingLatchDown, MovingPusherUp, MovingPusherDown, MovingLatchUp, SendingUp, SendingDown, Idling
//Module level functions: InitLoadServo, SendLoadServo
//Module defines: LATCH_SERVO, LATCH_DOWN, LATCH_DOWN_TIME, LATCH_UP, LATCH_UP_TIME, LATCH_SERVO_TIMER, PUSHER_SERVO, PUSHER_DOWN, PUSHER_DOWN_TIME, PUSHER_UP
//PUSHER_UP_TIME, PUSHER_SERVO_TIMER, LOAD_SERVO_UP, LOAD_SERVO_DOWN, LOAD_UP_TIME, LOAD_WAIT_TIME, LOAD_DOWN_TIME, LOAD_SERVO_TIMER

#define LOAD_SERVO_UP 180
#define LOAD_SERVO_DOWN 0
#define LOAD_UP_TIME 500
#define LOAD_WAIT_TIME 1000
#define LOAD_DOWN_TIME 500
#define LOAD_SERVO_TIMER 3


static uint8_t MyPriority;
static FiringState_t CurrentState;

static void InitLoadServo(void);
static void SendLoadServo(void);


bool InitFiringService(uint8_t Priority)
{
	// Initialize MyPriority to Priority
	MyPriority = Priority;
	//Initialize CurrentState to Waiting
	CurrentState = Waiting;
	
	//Initialize the PWM mode for the servos
	InitLoadServo();
	
	// Return true
	return true;
}



bool PostFiringService(ES_Event ThisEvent)
{
	// Return ThisEvent posted successfully to the service associated with MyPriority
	return ES_PostToService( MyPriority, ThisEvent);
}




ES_Event RunFiringService(ES_Event ThisEvent)
{
	// local variable NextState
	FiringState_t NextState;
	// local variable ReturnEvent
	ES_Event ReturnEvent;
	// Initialize ReturnEvent to ES_NO_EVENT
	ReturnEvent.EventType = ES_NO_EVENT;
	
	// Initialize NextState to CurrentState
	NextState = CurrentState;
	
	switch(CurrentState)
	{
		// If CurrentState is Waiting
		case Waiting:
		{
			// If ThisEvent is ES_FIRE
			if(ThisEvent.EventType == ES_FIRE)
			{
				// Send LOAD_SERVO to LOAD_SERVO_UP position
				SendLoadServo(LOAD_SERVO_UP);
				// Start LOAD_SERVO_TIMER for LOAD_UP_TIME
				ES_Timer_InitTimer(LOAD_SERVO_TIMER, LOAD_UP_TIME);
				// Set NextState to SendingUp
				NextState = SendingUp;
			}// EndIf
			break;
		} //End Waiting block
	
		// If CurrentState is SendingUp
		case SendingUp:
		{
			// If ThisEvent is ES_TIMEOUT
			if(ThisEvent.EventType == ES_TIMEOUT)
			{
				// Start LOAD_SERVO_TIMER for LOAD_WAIT_TIME
				ES_Timer_InitTimer(LOAD_SERVO_TIMER, LOAD_WAIT_TIME);
				// Set NextState to Idling
				NextState = Idling;
			}// EndIf
			break;
		}// End SendingUp block
		
		// If CurrentState is Idling
		case Idling:
		{
			// If ThisEvent is ES_TIMEOUT
			if(ThisEvent.EventType == ES_TIMEOUT)
			{
				// Send LOAD_SERVO to LOAD_SERVO_DOWN
				SendLoadServo(LOAD_SERVO_DOWN);
				// Start LOAD_SERVO_TIMER for LOAD_DOWN_TIME
				ES_Timer_InitTimer(LOAD_SERVO_TIMER, LOAD_DOWN_TIME);
				// Set NextState to SendingDown
				NextState = SendingDown;
			}// EndIf
			break;
		}// End Idling block
		

		// If CurrentState is SendingDown
		case SendingDown:
		{
			// If This is ES_TIMEOUT
			if(ThisEvent.EventType == ES_TIMEOUT)
			{
				//Post ES_FIRE_COMPLETE to MasterHSM
				ES_Event NewEvent;
				NewEvent.EventType == ES_FIRE_COMPLETE;
				PostMasterHSM(NewEvent);
				// Set NextState to Waiting
				NextState = Waiting;
			}// EndIf
			break;
		}// End SendingDown block

	} //end switch
	
	// Set CurrentState to NextState
	CurrentState = NextState;
	// Return ReturnEvent
	return ReturnEvent;
}

static void InitLoadServo(void)
{
/* CORRECT ALL OF THIS FOR PB0
	// Enable the clock to the PWM Module
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	
	// Enable the clock to Port B
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (GPIO_PIN_4 | GPIO_PIN_5);
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (GPIO_PIN_4 | GPIO_PIN_5);
	
	// Select the system clock/32
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) | (SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	
	// Make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0)
		;
	
	// Disable the PWM generator while initializing
	HWREG(PWM0_BASE+PWM_O_0_CTL) = 0;
	
	restoreA = true;
	restoreB = true;
	
	// Set generator to go up to 1 at rising A, 0 on falling A
	HWREG(PWM0_BASE+PWM_O_0_GENA) = GenA_Normal;
	
	// Set generator to go to 1 at rising B, 0 on falling B
	HWREG(PWM0_BASE+PWM_O_0_GENB) = GenB_Normal;
	
	// Set the load to ½ the desired period of 5 ms since going up and down
	HWREG(PWM0_BASE+PWM_O_0_LOAD) = ((PWM_LOAD_VALUE)) >> 1;
	
	// Set the initial duty cycle on A to 50%
	HWREG(PWM0_BASE+PWM_O_0_CMPA) = HWREG(PWM0_BASE+PWM_O_0_LOAD)>>1;
	
		// Set the initial duty cycle on B to 50%
	HWREG(PWM0_BASE+PWM_O_0_CMPB) = HWREG(PWM0_BASE+PWM_O_0_LOAD)>>1;
	
	// Enable the PWM outputs
	HWREG(PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);
	
	// Select the alternate function for PB6 and PB7
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT7HI | BIT6HI);
	
	// Choose to map PWM to those pins
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0X00ffffff) + (4<<(7*BitsPerNibble)) + (4<<(6*BitsPerNibble)); 
	
	// Enable pins 6 and 7 on Port B for digital outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (BIT7HI | BIT6HI);
	
	// Set the up/down count mode
	// Enable the PWM generator
	// Make generator updates locally synchronized to zero count
	HWREG(PWM0_BASE+PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE | PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);
               
*/
}

static void SendLoadServo(uint8_t position)
{
}

