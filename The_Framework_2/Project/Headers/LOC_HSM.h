/****************************************************************************
 Template header file for Hierarchical Sate Machines AKA StateCharts

 ****************************************************************************/

#ifndef LOC_HSM_H
#define LOC_HSM_H

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// State definitions for use with the query function
typedef enum {LOC_Waiting, LOC_Transmitting} LOC_State_t ;

// Public Function Prototypes

ES_Event RunLOC_SM( ES_Event CurrentEvent );
void StartLOC_SM ( ES_Event CurrentEvent );
bool PostLOC_SM( ES_Event ThisEvent );
bool InitLOC_SM ( uint8_t Priority );

#endif /*LOC_HSM_H */

