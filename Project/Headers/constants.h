/****************************************************************************
 Header file for keeping track of all constants

 ****************************************************************************/
 #define STATUS_COMMAND 0b1100 0000
 #define REPORT_COMMAND 0b1000 0000
 #define QUERY_RESPONSE_COMMAND 0b0111 0000
// 
// #define CHECK_SHOOT_GREEN getByteThree() & BIT7HI >> 7
// #define CHECK_SHOOT_RED getByteThree() & BIT3HI >> 3
 #define CHECK_IN 0
 #define SHOOT 1
 
// #define GREEN_STAGE_ACTIVE getByteThree() & (BIT4HI | BIT5HI | BIT6HI) >> 4
// #define GREEN_GOAL_ACTIVE getByteThree() & (BIT4HI | BIT5HI | BIT6HI) >> 4
// #define RED_STAGE_ACTIVE getByteThree() & (BIT0HI | BIT1HI | BIT2HI)
// #define RED_GOAL_ACTIVE getByteThree() & (BIT0HI | BIT1HI | BIT2HI)
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
 
// #define GREEN_SCORE getByteFour() & ~(BIT6HI | BIT7HI)
// #define RED_SCORE getByteFive() & ~(BIT6HI | BIT7HI) 
// 
// #define GAME_STATUS getByteFive() & BIT7HI
// 
// 
// #define RESPONSE_READY_STATUS getByteThree()
 #define RESPONSE_NOT_READY 0
 #define RESPOSE_READY 0xAA
 
// #define REPORT_STATUS getByteThree() & (BIT6HI | BIT7HI) 
 #define ACK 0
 #define NACK 0b10
 #define INACTIVE 0b11
 
 