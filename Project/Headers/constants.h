/****************************************************************************
 Header file for keeping track of all constants

 ****************************************************************************/
 #define CheckOff3 1
 #define SM_TEST 0
 
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
