// ConstructingState_t: GettingTargetStation, DrivingAlongTape, CheckIn, Shooting, Reloading
// Module level functions: DuringGettingTargetStation, DuringDrivingAlongTape, DuringCheckIn, DuringShooting, DuringReloading

#ifndef ConstructingSM_H
#define ConstructingSM_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// State definitions for use with the query function
typedef enum { GettingTargetStation, DrivingAlongTape, CheckIn, Shooting, AlignToTape, Reloading } ConstructingState_t ;

// Public Function Prototypes

ES_Event RunConstructingSM( ES_Event CurrentEvent );
void StartConstructingSM ( ES_Event CurrentEvent );
void UpdateStatus( void );
uint32_t getPeriod( void );
uint8_t getBallCount(void);
void SetBallCount(uint8_t count);
uint8_t incrementScore(void);
uint8_t getScore(void);
uint8_t getTargetGoal(void);
void setGameTimeoutFlag(bool flag);
bool getGameTimeoutFlag(void);
bool getHasLeftStage (void);
void setInitHallEffect(bool value);


#endif /*ConstructingSM_H */
