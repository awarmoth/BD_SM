#ifndef BumpSwitchDebounce_H
#define BumpSwitchDebounce_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */


typedef enum { Ready2SampleBump, DEBOUNCING_BUMP} BumpSwitchState_t ;
// Public Function Prototypes

bool InitializeBumpSwitchDebounce (uint8_t Priority);
bool PostBumpSwitchDebounce( ES_Event ThisEvent );
bool CheckBumpSwitchEvents(void);
ES_Event RunBumpSwitchDebounceSM (ES_Event ThisEvent);

#endif /* BumpSwitchDebounce_H */
