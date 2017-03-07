#ifndef TeamSwitchDebounce_H
#define TeamSwitchDebounce_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */


typedef enum { Ready2SampleTeam, DEBOUNCING_TEAM} TeamSwitchState_t ;
// Public Function Prototypes

bool InitializeTeamSwitchDebounce (uint8_t Priority);
bool PostTeamSwitchDebounce( ES_Event ThisEvent );
bool CheckTeamSwitchEvents(void);
ES_Event RunTeamSwitchDebounceSM (ES_Event ThisEvent);

#endif /* TeamSwitchDebounce_H */
