/****************************************************************************
 Header file for the ByteTransfer State Machine

 ****************************************************************************/

#ifndef BYTE_TRANSFER_SM
#define BYTE_TRANSFER_SM

// State definitions for use with the query function
typedef enum {BT_Wait2Start, BT_Wait4EOT, BT_Wait4Timeout} ByteTransferState_t ;

// Public Function Prototypes

ES_Event RunByteTransferSM(ES_Event CurrentEvent);
void StartByteTransferSM (ES_Event CurrentEvent);
uint8_t getByte2(void);
uint8_t getByte3(void);
uint8_t getByte4(void);
uint8_t getByte5(void);


#endif /*BYTE_TRANSFER_SM_H */

