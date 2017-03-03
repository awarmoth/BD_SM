// FFAState_t: WaitingFree4All, Driving2Reload, AlignFFA, RapidFiring, Firing, Reloading

#ifndef Free4AllSM_H
#define Free4AllSM_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// State definitions for use with the query function
typedef enum { WaitingFFA, Driving2ReloadFFA, AlignFFA, RapidFiringFFA, FiringFFA, ReloadingFFA } FFAState_t ;

// Public Function Prototypes

ES_Event RunFree4AllSM( ES_Event CurrentEvent );
void StartFree4AllSM ( ES_Event CurrentEvent );
static ES_Event DuringWaitingFFA( ES_Event Event);
static ES_Event DuringDriving2ReloadFFA( ES_Event Event);
static ES_Event DuringAlignFFA( ES_Event Event);
static ES_Event DuringRapidFiringFFA( ES_Event Event);
static ES_Event DuringFiringFFA( ES_Event Event);
static ES_Event DuringReloadingFFA(ES_Event ThisEvent);

#endif //Free4AllSM_H