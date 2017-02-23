/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef LOC_HSM_H
#define LOC_HSM_H

// State definitions for use with the query function
typedef enum { STATE_ONE, STATE_TWO, STATE_THREE } LOC_State_t ;

// Public Function Prototypes

ES_Event RunLOC_SM( ES_Event CurrentEvent );
void StartLOC_SM ( ES_Event CurrentEvent );
bool PostLOC_SM( ES_Event ThisEvent );
bool InitLOC_SM ( uint8_t Priority );

#endif /*LOC_HSM_H */

