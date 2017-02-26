/****************************************************************************
 Header file for keeping track of all constants

 ****************************************************************************/
 #define STATUS_COMMAND 0b1100 0000
 #define REPORT_COMMAND 0b1000 0000
 #define QUERY_RESPONSE_COMMAND 0b0111 0000

 #define GREEN 0
 #define RED 1

 #define CHECK_SHOOT_GREEN_MASK BIT7HI
 #define CHECK_SHOOT_GREEN_RIGHT_SHIFT 7

 #define CHECK_SHOOT_RED_MASK BIT3HI 
 #define CHECK_SHOOT_RED_RIGHT_SHIFT 3
 #define CHECK_IN 0
 #define SHOOT 1
 
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
 #define STAGE_ONE 1
 #define GOAL_ONE 1
 #define STAGE_TWO 0b10
 #define GOAL_TWO 0b10
 #define STAGE_THREE 0b11
 #define GOAL_THREE 0b11
 #define ALL_GOALS_ACTIVE 0b111
 #define ALL_STAGES_ACTIVE 0b111
 
 #define GREEN_SCORE_MASK ~(BIT6HI | BIT7HI)
 #define RED_SCORE_MASK ~(BIT6HI | BIT7HI) 

 #define GAME_STATUS_MASK BIT7HI
 #define GAME_STATUS_RIGHT_SHIFT 7
 #define REPORT_STATUS_MASK (BIT6HI | BIT7HI)
 #define REPORT_STATUS_RIGHT_SHIFT 6

 #define RELOAD 0
 #define STATION_ONE 1
 #define STATION_TWO 2
 #define STATION_THREE 3
 #define START 4

 #define GOAL_ONE 1
 #define GOAL_TWO 2
 #define GOAL_THREE 3
 
 