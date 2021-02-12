//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef CAN_TX_MANAGER_H
#define CAN_TX_MANAGER_H

#include "IOExpander.h"
#include "MessageBuilder.h"

//-----------------------------------------------------------------------------
// typedef
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define IOEXPANDER_MESSAGE_BUFFER_SIZE		200

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

extern void CANTxManager_Init(void);
extern void CANTxManager_Reset(void);
extern void CANTxManager_AddMessage(IOExpanderMsg_t * Msg);
extern void CANTxManager_AckReceived(IOExpanderMsg_t * Msg);
extern void CANTxManager_SendWrappedRxDataPacket(uint8_t * pbData, uint8_t bLength);
extern void CANTxManager_SendIoxInformationStatus(tIoxInformationStatus Type, uint8_t * pData, uint8_t bLength);
extern void CANTxManager_SendExternalDeviceConnected(uint16_t usDeviceID, uint16_t usFlags, uint8_t bConnected);
extern void CANTxManager_BuildMultiFrameMessage(tMultiFrameLogTypes LogType, const uint8_t * pbData, uint16_t usLength);
extern void CANTxManager_SendRxWakeup(void);
extern void CANTxManager_SendRxData(const uint8_t * pbData, uint8_t bLength);
extern void CANTxManager_SaveGenericDataRecord(uint16_t usDataID, uint8_t bLen, uint32_t ulData);

#endif	// CAN_TX_MANAGER_H

// End of file
