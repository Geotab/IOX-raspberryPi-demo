//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef MB_H
#define MB_H

#include "IOExpander.h"
#include "can.h"

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef enum
{
	SINGLE_FRAME_DATA_GENERIC_DATA_RECORD = 0,
	SINGLE_FRAME_DATA_EXTERNAL_DEVICE_CONNECTION_STATUS = 1,
	SINGLE_FRAME_DATA_GENERIC_FAULT_RECORD = 2,
	SINGLE_FRAME_DATA_PRIORITY_DATA_RECORD = 3,
} tSingleFrameLogTypes;

typedef enum
{
	MULTI_FRAME_DATA_THIRD_PARTY_FREE_FORMAT = 0,
	MULTI_FRAME_DATA_HOS_INFO_MESSAGE = 1,
	MULTI_FRAME_DATA_BLUETOOTH_RECORD = 2,
} tMultiFrameLogTypes;

typedef enum
{
	// Information Type for IOX Request/Status (0x25)
	IOX_STATUS_BUSY = 0,
	IOX_STATUS_PACKET_WRAPPER = 1,
	IOX_STATUS_REQUEST_GO_DATA = 2,
	IOX_STATUS_CONNECT_AND_SEND = 3,
	IOX_STATUS_REQUEST_VIN_MSG = 4,			// Currently only used by a third party IOX
} tIoxInformationStatus;

typedef struct __attribute__((packed))
{
	uint16_t usDataId;
	uint8_t bLength;
	uint8_t abData[4];
}
tGenericDataRecord;

typedef struct __attribute__((packed))
{
	uint8_t bConnected;
	uint16_t usDataId;
	uint16_t usFlags;
}
tExternalDeviceConnectionStatus;

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

extern void MB_PollResponse(IOExpanderMsg_t * Msg, bool fFirstPoll, bool fGoingToSleep, bool fJustWokeUp, uint8_t bResetReason, bool fACKRequired);
extern void MB_AdditionalInfoMsg(IOExpanderMsg_t * Msg, uint32_t iSVNVersion, uint8_t bProductVerion, uint8_t bErrorCondition, uint8_t bHardwareVersion, bool fACKRequired);
extern void MB_RxDataMsg(IOExpanderMsg_t * Msg, const uint8_t * pbData, uint8_t bLength, bool fACKRequired);
extern void MB_SingleFrameLogData(IOExpanderMsg_t * Msg, uint8_t bLogType, const uint8_t * pbData, uint8_t bLength);
extern void MB_MultiFrameLogData(IOExpanderMsg_t * Msg, uint8_t bFrameCounter, const uint8_t * pbData, uint8_t bLength);
extern void MB_IoxStatus(IOExpanderMsg_t * Msg, uint16_t uiType, uint8_t * pbData, uint8_t bLength);
extern void MB_ConvertToIOExpanderMsg(IOExpanderMsg_t * IOExpanderMsg, CANMsg_t * CANMsg);
extern void MB_ConvertToCANMsg(CANMsg_t * CANMsg, IOExpanderMsg_t * IOExpanderMsg);

#endif	// MB_H

// End of file
