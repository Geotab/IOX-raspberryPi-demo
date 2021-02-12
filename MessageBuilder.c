//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
// Module Notes:
// =============
//
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "Types.h"
#include "IOExpander.h"
#include "MessageBuilder.h"
#include "string.h"
#include "can.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

void MB_PollResponse(IOExpanderMsg_t * Msg, bool fFirstPoll, bool fGoingToSleep, bool fJustWokeUp, uint8_t bResetReason, bool fACKRequired)
{
	ByteBits_t bSatusFlags;

	Msg->bLength = 0x01;
	Msg->bMsgType = MSG_ID_POLL_RESPONSE;
	Msg->iSerialNumber = gIox.ulSerialNumber;
	Msg->fACKRequired = fACKRequired;

	//Update the first byte of data to be sent
	bSatusFlags.bValue = 0x00;
	bSatusFlags.Bits.b0 = fFirstPoll;
	bSatusFlags.Bits.b1 = fGoingToSleep;
	bSatusFlags.Bits.b2 = fJustWokeUp;

	Msg->abData[0] = bSatusFlags.bValue;

	//Build message for the first poll response
	if (fFirstPoll)
	{
		Msg->bLength = 0x08;

		Msg->abData[1] = FW_VERSION_MAJOR;
		Msg->abData[2] = FW_VERSION_MINOR;
		Msg->abData[3] = (gIox.ulSerialNumber >> 24);
		Msg->abData[4] = (gIox.ulSerialNumber >> 16);

		Msg->abData[5] = bResetReason;
		Msg->abData[6] = 0;
		Msg->abData[7] = gIox.bIoxType;
	}
}

void MB_AdditionalInfoMsg(IOExpanderMsg_t * Msg, uint32_t iSVNVersion, uint8_t bProductVerion, uint8_t bErrorCondition, uint8_t bHardwareVersion, bool fACKRequired)
{
	Msg->bLength = 0x08;
	Msg->bMsgType = MSG_ID_ADDITIONAL_INFO;
	Msg->iSerialNumber = gIox.ulSerialNumber;
	Msg->fACKRequired = fACKRequired;

	Msg->abData[0] = ((iSVNVersion >> 24) & 0xFF);
	Msg->abData[1] = ((iSVNVersion >> 16) & 0xFF);
	Msg->abData[2] = ((iSVNVersion >> 8) & 0xFF);
	Msg->abData[3] = ((iSVNVersion >> 0) & 0xFF);

	Msg->abData[4] = bProductVerion;

	Msg->abData[5] = 0;

	Msg->abData[6] = bErrorCondition;

	Msg->abData[7] = bHardwareVersion;
}

void MB_RxDataMsg(IOExpanderMsg_t * Msg, const uint8_t * pbData, uint8_t bLength, bool fACKRequired)
{
	Msg->bLength = bLength;
	Msg->bMsgType = MSG_ID_RX_DATA;
	Msg->iSerialNumber = gIox.ulSerialNumber;
	Msg->fACKRequired = fACKRequired;

	memcpy(Msg->abData, pbData, bLength);
}

void MB_SingleFrameLogData(IOExpanderMsg_t * Msg, uint8_t bLogType, const uint8_t * pbData, uint8_t bLength)
{
	Msg->bLength = bLength + 1;
	Msg->bMsgType = MSG_ID_SINGLE_FRAME_LOG_DATA;
	Msg->iSerialNumber = gIox.ulSerialNumber;
	Msg->fACKRequired = true;

	Msg->abData[0] = bLogType;
	memcpy(&Msg->abData[1], pbData, bLength);
}

void MB_MultiFrameLogData(IOExpanderMsg_t * Msg, uint8_t bFrameCounter, const uint8_t * pbData, uint8_t bLength)
{
	Msg->bLength = bLength + 1;
	Msg->bMsgType = MSG_ID_MULTI_FRAME_LOG_DATA;
	Msg->iSerialNumber = gIox.ulSerialNumber;
	Msg->fACKRequired = true;

	Msg->abData[0] = bFrameCounter;
	memcpy(&Msg->abData[1], pbData, bLength);
}

/**
 * MB_IoxStatus:
 * To build the IOX Request/Status (0x25) message going to GO device.
 * @param Msg     = Message structure to contain the IOX Request/Status (0x25) message.
 * @param uiType  = Information-Type of the Request/Status message (Refer to tIoxInformationStatus).
 * @param pbData  = Data array to copy to Msg.
 * @param bLength = # bytes in pbData[] to copy (0 to 6).
 */
void MB_IoxStatus(IOExpanderMsg_t * Msg, uint16_t uiType, uint8_t * pbData, uint8_t bLength)
{
	bLength  = min(bLength, 6);

	Msg->bLength = bLength + 2;
	Msg->bMsgType = MSG_ID_IOX_STATUS_INFORMATION;
	Msg->iSerialNumber = gIox.ulSerialNumber;
	Msg->fACKRequired = true;

	memcpy(Msg->abData, &uiType, 2);
	memcpy(&Msg->abData[2], pbData, bLength);
}

void MB_ConvertToIOExpanderMsg(IOExpanderMsg_t * IOExpanderMsg, CANMsg_t * CANMsg)
{
	IOExpanderMsg->bLength = CANMsg->can_dlc;

	IOExpanderMsg->bMsgType = ((CANMsg->can_id & IO_EXPANDER_COMMAND_MASK) >> 16);

	IOExpanderMsg->iSerialNumber = (uint16_t)((CANMsg->can_id & IO_EXPANDER_ID_MASK));

	memcpy(IOExpanderMsg->abData, &CANMsg->data, CANMsg->can_dlc); //Payload.abData
}

void MB_ConvertToCANMsg(CANMsg_t * CANMsg, IOExpanderMsg_t * IOExpanderMsg)
{
	CANMsg->can_dlc = IOExpanderMsg->bLength;

	CANMsg->can_id = CAN_EFF_FLAG;
	CANMsg->can_id |= IOExpanderMsg->iSerialNumber;

	CANMsg->can_id |= (IOExpanderMsg->bMsgType << 16);

	memcpy(&CANMsg->data, IOExpanderMsg->abData, IOExpanderMsg->bLength);
}

// End of file
