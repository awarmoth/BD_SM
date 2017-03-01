//ShootingState_t: AlignToGoal, Firing, WaitForShotResult, WaitForScoreUpdate, AlignToTape
//Module level functions: DuringAlignToGoal, DuringFiring, DuringWaitForShotResult, DuringWaitForScoreUpdate

#ifndef ShootingSM_H
#define ShootingSM_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum { AlignToGoal, Firing, WaitForShotResult, WaitForScoreUpdate, AlignToTape } ShootingState_t ;

// Public Function Prototypes

ES_Event RunShootingSM( ES_Event CurrentEvent );
void StartShootingSM ( ES_Event CurrentEvent );
ES_Event DuringAlignToGoal ( ES_Event ThisEvent );
ES_Event DuringFiring ( ES_Event ThisEvent );
ES_Event DuringWaitForShotResult ( ES_Event ThisEvent );
ES_Event DuringWaitForScoreUpdate ( ES_Event ThisEvent );
ES_Event DuringAlignToTape ( ES_Event ThisEvent );
uint8_t getPeriodCode(uint32_t Period);
void ClearBadResponseCounter(void);

#endif /*ShootingSM_H */
