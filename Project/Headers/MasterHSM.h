// MasterState_t: Waiting2Start, Constructing, Free4All, GameComplete
// Module level functions: DuringWaiting2Start, DuringConstructing, DuringFree4All, DuringGameComplete

#ifndef MasterHSM_H
#define MasterHSM_H

// State definitions for use with the query function
typedef enum { Waiting2Start, Constructing, Free4All, GameComplete } MasterState_t ;

// Public Function Prototypes

ES_Event RunMasterSM( ES_Event CurrentEvent );
void StartMasterSM ( ES_Event CurrentEvent );
bool PostMasterSM( ES_Event ThisEvent );
bool InitMasterSM ( uint8_t Priority );
static ES_Event DuringWaiting2Start( ES_Event Event);
static ES_Event DuringConstructing( ES_Event Event);
static ES_Event DuringFree4All( ES_Event Event);
static ES_Event DuringGameComplete( ES_Event Event)

#endif /*MasterHSM_H */
