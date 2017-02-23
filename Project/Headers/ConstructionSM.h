/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts
 02/08/12 adjsutments for use with the Events and Services Framework Gen2
 3/17/09  Fixed prototpyes to use Event_t
 ****************************************************************************/

#ifndef ConstructionSM_H
#define ConstructionSM_H


// typedefs for the states
// State definitions for use with the query function
typedef enum { STATE_ZERO, STATE_ONE, STATE_TWO } ConstructionState_t ;


// Public Function Prototypes

ES_Event RunConstructionSM( ES_Event CurrentEvent );
void StartConstructionSM ( ES_Event CurrentEvent );
ConstructionState_t QueryConstructionSM ( void );

#endif /*ConstructionSM_H */

