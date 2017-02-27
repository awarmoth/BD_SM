/****************************************************************************
 Header file for keeping track of all constants

 ****************************************************************************/
 #define CheckOff3 1
 #define SM_TEST 1
 #define TEAM_COLOR RED
 #define LOC_TEST 0
 
 #define GAME_TIMEOUT 20*1000 // change to 105? *1000
 
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
 
// Masks
 #define GREEN_SCORE_MASK ~(BIT6HI | BIT7HI)
 #define RED_SCORE_MASK ~(BIT6HI | BIT7HI) 
 #define GAME_STATUS_MASK BIT7HI
 #define GAME_STATUS_RIGHT_SHIFT 7
 #define REPORT_STATUS_MASK (BIT6HI | BIT7HI)
 #define REPORT_STATUS_RIGHT_SHIFT 6

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
 #define ONE_SHOT_TIMEOUT 2000000
 
 // Magnetic sensors
 #define RUN_AVERAGE_LENGTH 30
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
#define LEFT_MAX_DUTY 40
#define RIGHT_MAX_DUTY 40
#define COMMAND_DIFF 0
#define CONTROLLER_OFF 0
#define VELOCITY_CONTROLLER 1
#define POSITION_CONTROLLER 2

#define CONTROLLER_TIME_US 2000
#define TICKS_PER_US 40
