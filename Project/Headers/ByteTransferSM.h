/****************************************************************************
 Header file for the ByteTransfer State Machine

 ****************************************************************************/

#ifndef BYTE_TRANSFER_SM
#define BYTE_TRANSFER_SM

#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// State definitions for use with the query function
typedef enum {BT_Wait2Start, BT_Wait4EOT, BT_Wait4Timeout} ByteTransferState_t ;

// Public Function Prototypes

ES_Event RunByteTransferSM(ES_Event CurrentEvent);
void StartByteTransferSM (ES_Event CurrentEvent);
uint8_t getByte2(void);
uint8_t getByte3(void);
uint8_t getByte4(void);
uint8_t getByte5(void);
uint8_t getSB1_Byte(void);
uint8_t getSB2_Byte(void);
uint8_t getSB3_Byte(void);
uint8_t getRS_Byte(void);
uint8_t getRR_Byte(void);

#endif /*BYTE_TRANSFER_SM_H */

