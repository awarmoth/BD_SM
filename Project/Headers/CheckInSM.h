// CheckInState_t: Reporting_1, WaitForResponse_1, Reporting_2, WaitForResponse_2
// Module level functions: DuringReporting_1, DuringWaitForResponse_1, DuringReporting_2, DuringWaitForResponse_2

#ifndef CheckInSM_H
#define CheckInSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { Reporting_1, WaitForResponse_1, Reporting_2, WaitForResponse_2 } CheckInState_t ;

// Public Function Prototypes

ES_Event RunCheckInSM( ES_Event CurrentEvent );
void StartCheckInSM ( ES_Event CurrentEvent );
//ES_Event DuringReporting_1 ( ES_Event ThisEvent );
//ES_Event DuringWaitForResponse_1 ( ES_Event ThisEvent );
//ES_Event DuringReporting_2 ( ES_Event ThisEvent );
//ES_Event DuringWaitForResponse_2 ( ES_Event ThisEvent );
uint8_t getPeriodCode(uint32_t Period);
void ClearBadResponseCounter(void);

#endif /*CheckInSM_H */

