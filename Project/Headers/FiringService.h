#ifndef FIRING_SERVICE_H
#define FIRING_SERVICE_H

bool InitFiringService(uint8_t Priority);
bool PostFiringService(ES_Event ThisEvent);
ES_Event RunFiringService(ES_Event ThisEvent);

typedef enum {Waiting, MovingLatchDown, MovingPusherUp, MovingLatchUp, MovingPusherDown} FiringState_t ;


#endif /*FIRING_SERVICE_H*/
