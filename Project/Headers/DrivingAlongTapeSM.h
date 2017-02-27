#ifndef DRIVING_ALONG_TAPE_SM_H
#define DRIVING_ALONG_TAPE_SM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { Waiting, Driving2Station, Driving2Reload} DrivingState_t ;


// Public Function Prototypes

ES_Event RunDrivingAlongTapeSM( ES_Event CurrentEvent );
void StartDrivingAlongTapeSM ( ES_Event CurrentEvent );
void SetController(uint8_t control);

#endif /*DRIVING_ALONG_TAPE_SM_H */
