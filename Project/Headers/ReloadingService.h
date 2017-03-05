//ReloadState_t: WaitingReload, Emitting_High, Emitting_Low, Wait4Delivery

#ifndef RELOADING_SERVICE_H
#define RELOADING_SERVICE_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

bool InitReloadingService(uint8_t Priority);
bool PostReloadingService(ES_Event ThisEvent);
ES_Event RunReloadingService(ES_Event ThisEvent);

typedef enum {WaitingReload, Emitting_High, Emitting_Low, Wait4Delivery} ReloadState_t ;


#endif /*FIRING_SERVICE_H*/
