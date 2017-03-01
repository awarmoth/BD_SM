module level variables: MyPriority, CurrentState
FiringState_t: Waiting, MovingLatchDown, MovingPusherUp, MovingPusherDown, MovingLatchUp
Module level functions:
Module defines: LATCH_SERVO, LATCH_DOWN, LATCH_DOWN_TIME, LATCH_UP, LATCH_UP_TIME, LATCH_SERVO_TIMER, PUSHER_SERVO, PUSHER_DOWN, PUSHER_DOWN_TIME, PUSHER_UP
PUSHER_UP_TIME, PUSHER_SERVO_TIMER

bool InitFiringService(uint8_t Priority)
{
	// local variable ThisEvent
	// Initialize MyPriority to Priority
	//Initialize CurrentState to Waiting
	
	// Return true
}



bool PostFiringService(ES_Event ThisEvent)
{
	// Return ThisEvent posted successfully to the service associated with MyPriority
}




ES_Event RunFiringService(ES_Event ThisEvent)
{
	// local variable NextState
	// local variable ReturnEvent
	// Initialize ReturnEvent to ES_NO_EVENT
	
	// Initialize NextState to CurrentState
	
	// If CurrentState is Waiting
		// If ThisEvent is ES_START_FLYWHEEL
			// Start ramping the flywheel to the correct velocity based upon staging area and target goal
		// EndIf
		
		// If ThisEvent is ES_BRAKE_FLYWHEEL
			// Turn off power to the flywheel
		// EndIf
		
		// If ThisEvent is ES_FIRE
			// Send LATCH_SERVO to LATCH_DOWN position
			// Start LATCH_SERVO_TIMER for LATCH_DOWN_TIME
			// Set NextState to MovingLatchDown
		// EndIf
	// EndIf
	
	// If CurrentState is MovingLatchDown
		// If ThisEvent is ES_TIMEOUT and the timer is LATCH_SERVO_TIMER
			// Send PUSHER_SERVO to PUSHER_UP position
			// Start PUSHER_SERVO_TIMER for PUSHER_UP_TIME
			// Set NextState to MovingPusherUp
		// EndIf
	// End MovingLatchDown block
	
	// If CurrentState is MovingPusherUp
		// If ThisEvent is ES_TIMEOUT and the timer is PUSHER_SERVO_TIMER
			// Send LATCH_SERVO to LATCH_UP position
			// Start LATCH_SERVO_TIMER for LATCH_UP_TIME
			// Set NextState to MovingLatchUp
		// EndIf
	// End MovingPusherUp block
	
	// If CurrentState is MovingLatchUp
		// If This is ES_TIMEOUT and the timer is LATCH_SERVO_TIMER
			// Send PUSHER_SERVO to PUSHER_DOWN position
			// Start PUSHER_SERVO_TIMER for PUSHER_DOWN_TIME
			// Set NextState to MovingPusherDown
		// EndIf
	// End MovingLatchUp block
	
	// If CurrentState is MovingPusherDown
		// If ThisEvent is ES_TIMEOUT and the timer is PUSHER_SERVO_TIMER
			// Post ES_FIRE_COMPLETE to MasterHSM
			// Set NextState to Waiting
		// EndIf
	// End MovingPusherDown block
	
	
	// Set CurrentState to NextState
	// Return ReturnEvent
}



