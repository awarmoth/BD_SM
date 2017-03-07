#ifndef FIRING_SERVICE_H
#define FIRING_SERVICE_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

bool InitFiringService(uint8_t Priority);
bool PostFiringService(ES_Event ThisEvent);
ES_Event RunFiringService(ES_Event ThisEvent);

typedef enum {WaitingFire, SendingUp, SendingDown, Idling} FiringState_t ;


#endif /*FIRING_SERVICE_H*/
