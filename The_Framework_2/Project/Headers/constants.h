/****************************************************************************
 Header file for keeping track of all constants

 ****************************************************************************/
 #define CheckOff3 0
 #define SM_TEST 1
 #define TEAM_COLOR RED
 #define NO_LOC 0
 #define BALL_TRACKING 0
 #define TAPE_TEST 0
 
 #define GAME_TIMEOUT 20*1000 // change to 105? *1000 (need to use interrupt 32-bit wide timer: correspondes to 107 seconds
 
 // Commands
 #define STATUS_COMMAND 0xc0
 #define REPORT_COMMAND 0x80
 #define QUERY_RESPONSE_COMMAND 0x70

// Colors
 #define GREEN 0
 #define RED 1

// Game Start
 #define WAITING_FOR_START 0
 #define GAME_STARTED 1

// Check In
 #define CHECK_SHOOT_GREEN_MASK BIT7HI
 #define CHECK_SHOOT_GREEN_RIGHT_SHIFT 7
 #define CHECK_SHOOT_RED_MASK BIT3HI 
 #define CHECK_SHOOT_RED_RIGHT_SHIFT 3
 #define CHECK_IN 0
 #define SHOOT 1
 
// Board portions active
 #define GREEN_STAGE_ACTIVE_MASK (BIT4HI | BIT5HI | BIT6HI)
 #define GREEN_STAGE_ACTIVE_RIGHT_SHIFT 4
 #define GREEN_GOAL_ACTIVE_MASK (BIT4HI | BIT5HI | BIT6HI)
 #define GREEN_GOAL_ACTIVE_RIGHT_SHIFT 4
 #define RED_STAGE_ACTIVE_MASK (BIT0HI | BIT1HI | BIT2HI)
 #define RED_STAGE_ACTIVE_RIGHT_SHIFT 0
 #define RED_GOAL_ACTIVE_MASK (BIT0HI | BIT1HI | BIT2HI)
 #define RED_GOAL_ACTIVE_RIGHT_SHIFT 0
 #define NO_GOAL_ACTIVE 0
 #define NO_STAGE_ACTIVE 0
 #define STAGE_ONE_ACTIVE 1
 #define GOAL_ONE_ACTIVE 1
 #define STAGE_TWO_ACTIVE 0x02
 #define GOAL_TWO_ACTIVE 0x02
 #define STAGE_THREE_ACTIVE 0x03
 #define GOAL_THREE_ACTIVE 0x03
 #define ALL_GOALS_ACTIVE 0x07
 #define ALL_STAGES_ACTIVE 0x07
 
 #define MAX_BAD_RESPONSES 4
 #define RESPONSE_NOT_READY 0
 #define RESPONSE_READY 0xAA
 
 #define ACK 0
 #define NACK 0x03
 #define INACTIVE 0x02
 
// Masks
 #define GREEN_SCORE_MASK ~(BIT6HI | BIT7HI)
 #define RED_SCORE_MASK ~(BIT6HI | BIT7HI) 
 #define GAME_STATUS_MASK BIT7HI
 #define GAME_STATUS_RIGHT_SHIFT 7
 #define REPORT_STATUS_MASK (BIT6HI | BIT7HI)
 #define REPORT_STATUS_RIGHT_SHIFT 6
 #define REPORT_ACK 0
 #define REPORT_INACTIVE (BIT7HI)
 #define REPORT_NACK (BIT6HI | BIT7HI)

// Locations
 #define LOCATION_MASK (BIT3HI | BIT2HI | BIT1HI | BIT0HI)
 #define RELOAD 0
 #define STATION_ONE 1
 #define STATION_TWO 2
 #define STATION_THREE 3
 #define START 4

// Goals
 #define GOAL_ONE 1
 #define GOAL_TWO 2
 #define GOAL_THREE 3
 
 // Timing
 #define ONE_SHOT_TIMEOUT 20000000
 
 // Magnetic sensors
 #define RUN_AVERAGE_LENGTH 15
 #define MAX_THROWAWAY 3
 #define TICKS_PER_USEC 40
 #define MAX_ALLOWABLE_PER (TICKS_PER_USEC*1361)
 #define MIN_ALLOWABLE_PER (TICKS_PER_USEC*472)
 
 // define constants for the states for this machine
// and any other local defines
#define INITIAL_STATION 4 //dummy station for when the robot is at the start location
#define STAGING_AREA_1 1
#define STAGING_AREA_2 2
#define STAGING_AREA_3 3
#define SUPPLY_DEPOT 0
#define FORWARD 1
#define REVERSE -1
#define TICKS_PER_MS 40000

//define controller constants
#define LEFT_MAX_DUTY 60
#define RIGHT_MAX_DUTY 60
#define RIGHT_CCW_COMMAND 35
#define RIGHT_CW_COMMAND 35
#define LEFT_CCW_COMMAND 35
#define LEFT_CW_COMMAND 35
#define COMMAND_DIFF 0

#define RIGHT_CCW_DIR 1
#define RIGHT_CW_DIR 0
#define LEFT_CCW_DIR 0
#define LEFT_CW_DIR 1
#define FORWARD_DIR 0
#define REVERSE_DIR 1

#define CONTROLLER_OFF 0
#define VELOCITY_CONTROLLER 1
#define POSITION_CONTROLLER 2

#define STOP_DRIVING 0
#define ROTATE_CCW 1
#define ROTATE_CW 2
#define DRIVE_ON_TAPE_FORWARD 3
#define DRIVE_ON_TAPE_REVERSE 4

#define FORWARD_RIGHT_RESONANCE_AD 0
#define FORWARD_LEFT_RESONANCE_AD 1
#define REVERSE_RIGHT_RESONANCE_AD 2
#define REVERSE_LEFT_RESONANCE_AD 3

#define TAPE_WATCH_WINDOW 20
#define TAPE_THRESHOLD 20
#define MOTOR_CONTROLLER_TIME_US 10000

#define TICKS_PER_US 40
#define TICKS_PER_S 40000000
#define S_PER_MIN 60

#define LAUNCHER_PULSE_PER_REV 3
#define LAUNCHER_CONTROLLER_TIME_US 2000

//define ball counters
#define BALL_START_COUNT 3
#define NO_BALLS 0
#define MAX_BALLS 5

#define WARM_UP_TIME 2000
#define SHOT_CLOCK_TIME 20000
#define BALL_AIR_TIME 4000 //will probably want to lower this

//Servo motor constants
#define NUM_MOTOR 1
#define MIN_MOT_POS 1700
#define MAX_MOT_POS 3200
#define MOT_FREQ 25000
#define MOTOR_PERIOD 		250
#define TIMING_CHANNEL 	0
#define TIME_MOT_GROUP 	0


#define NUM_PULSES 10
#define DELIVERY_TIME 3000
#define PULSE_HIGH_TIME 10
#define PULSE_LOW_TIME 30

#define BUMP_SENSOR_MASK (BIT4HI)
#define BUMP_SENSOR_SHIFT 4
#define TEAM_COLOR_MASK (BIT5HI)
#define TEAM_COLOR_SHIFT 5

