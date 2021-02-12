//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
// Module Notes:
// =============
//
// Notes
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "Types.h"
#include "string.h"
#include "IOExpander.h"
#include "NVM.h"
#include "statemachine.h"
#include "ticktimer.h"
#include "can.h"
#include "CANTxManager.h"
#include "IOControl.h"
#include "PRNG.h"
#include "InfoTable.h"
#include <linux/can.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef union
{
	uint8_t		bValue;

	struct
	{
		uint8_t	fACKReceived			: 1;
		uint8_t	fWaitForACK				: 1;
		uint8_t	fDelaySerialSending		: 1;
		uint8_t	fBusy					: 1;

		uint8_t						: 3;
	} Single;

}	CANTxFlags_t;

typedef struct
{
	IOExpanderMsg_t			Messages[IOEXPANDER_MESSAGE_BUFFER_SIZE];
	uint8_t					bHead;
	uint8_t					bTail;
	uint8_t					bCount;

} IOExpanderMessageBuffer_t;

typedef struct
{
	CANTxFlags_t				Flags;

	IOExpanderMessageBuffer_t	MessageBuffer;
	CANMsg_t					MessageToSend;

	Timer_t						Timer;

	uint32_t					iACKTimeout;

	uint8_t 					bACKType;
} CANTx_t;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

extern uint8_t gfGoHandshakeAcked;

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------

static CANTx_t mCANTx;

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

static void CANTxManager_GetNextMessage(IOExpanderMsg_t * Msg);
static void CANTxManager_SMIdle(bool fIsInit);
static void CANTxManager_SMSendMessage(bool fIsInit);
static void CANTxManager_SMWaitForACK(bool fIsInit);
static void CANTxManager_CheckMessageType(CANTx_t *pCANTxData);
//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : CANTxManager_Init
// Purpose    : Initialises the CANTxManager module
// Parameters : None
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void CANTxManager_Init(void)
{
	SM_Add(CANTxManager_SMIdle);

	mCANTx.iACKTimeout = CANTX_ACK_TIMEOUT_MAX_MS;
}

//-----------------------------------------------------------------------------
// Function   : CANTxManager_Reset
// Purpose    : Resets the buffers and flags of the CAN TX Manager
// Parameters : None
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void CANTxManager_Reset(void)
{
	mCANTx.Flags.bValue = 0x00;

	mCANTx.MessageBuffer.bTail = 0;
	mCANTx.MessageBuffer.bHead = 0;
	mCANTx.MessageBuffer.bCount = 0;
}

//-----------------------------------------------------------------------------
// Function   : CANTxManager_AddMessage
// Purpose    : Adds the IOX message to the TX Buffer
// Parameters : [I] Msg: IOX message to send
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void CANTxManager_AddMessage(IOExpanderMsg_t * Msg)
{
	memcpy((uint8_t *) &mCANTx.MessageBuffer.Messages[mCANTx.MessageBuffer.bHead], Msg, sizeof(IOExpanderMsg_t));

	mCANTx.MessageBuffer.bHead++;
	mCANTx.MessageBuffer.bHead %= IOEXPANDER_MESSAGE_BUFFER_SIZE;	// Wraparound head if neccessary

	if (mCANTx.MessageBuffer.bHead == mCANTx.MessageBuffer.bTail)
	{
		mCANTx.MessageBuffer.bTail++;		// Oldest message overwritten
		mCANTx.MessageBuffer.bTail %= IOEXPANDER_MESSAGE_BUFFER_SIZE;
	}
	else
	{
		mCANTx.MessageBuffer.bCount++;		// No overwrite, so increase count
	}
}

//-----------------------------------------------------------------------------
// Function   : CANTxManager_BuildMultiFrameMessage
// Purpose    : Build the multi-frame messages and add them to the transmit buffer
// Parameters : bLogType, pbData, usLength
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void CANTxManager_BuildMultiFrameMessage(tMultiFrameLogTypes LogType, const uint8_t * pbData, uint16_t usLength)
{
	IOExpanderMsg_t Msg;
	uint8_t bFrameCounter = 0;
	uint8_t abData[7];
	uint8_t bLength;

	abData[0] = LogType;
	memcpy(&abData[1], &usLength, sizeof(usLength));
	memcpy(&abData[3], pbData, 4);

	bLength = usLength;

	if (usLength > 4)
	{
		bLength = 4;
	}

	MB_MultiFrameLogData(&Msg, bFrameCounter, abData, bLength + 3);
	CANTxManager_AddMessage(&Msg);

	usLength -= bLength;
	pbData += bLength;

	while (usLength > 0)
	{
		bLength = usLength;

		if (usLength > 7)
		{
			bLength = 7;
		}

		bFrameCounter++;
		MB_MultiFrameLogData(&Msg, bFrameCounter, pbData, bLength);
		CANTxManager_AddMessage(&Msg);

		usLength -= bLength;
		pbData += bLength;
	}
}

void CANTxManager_SaveGenericDataRecord(uint16_t usDataID, uint8_t bLen, uint32_t ulData)
{
	// Send generic data to GoDevice for logging
	tGenericDataRecord genericRecord;
	IOExpanderMsg_t Msg;

	genericRecord.usDataId = usDataID;
	genericRecord.bLength = min(4, bLen);
	memcpy(genericRecord.abData, &ulData, sizeof(ulData));
	MB_SingleFrameLogData(&Msg, SINGLE_FRAME_DATA_GENERIC_DATA_RECORD, (uint8_t *)&genericRecord, sizeof(genericRecord));
	CANTxManager_AddMessage(&Msg);
}

/**
 * CANTxManager_SendIoxInformationStatus:
 * To send the IOX Request/Status (0x25) message to GO.
 * @param Type : Information Type.
 * @param bData: 1-byte of data.
 * @bLength: size of data
 * Refer to IO Expander Messages document - IOX Request/Status (0x25)
 */
void CANTxManager_SendIoxInformationStatus(tIoxInformationStatus Type, uint8_t * pData, uint8_t bLength)
{
	IOExpanderMsg_t Msg;
	MB_IoxStatus(&Msg, Type, pData, bLength);
	CANTxManager_AddMessage(&Msg);
}

//-----------------------------------------------------------------------------
// Build and send Rx wakeup packet to the GO
//-----------------------------------------------------------------------------
void CANTxManager_SendRxWakeup(void)
{
	IOExpanderMsg_t Msg;

	MB_RxDataMsg(&Msg, NULL, 0, false);	// Empty message used for wakeup

	CANTxManager_AddMessage(&Msg);
}

//-----------------------------------------------------------------------------
// Build and send Rx data packet to the GO
//-----------------------------------------------------------------------------
void CANTxManager_SendRxData(const uint8_t * pbData, uint8_t bLength)
{
	IOExpanderMsg_t Msg;

	MB_RxDataMsg(&Msg, pbData, bLength, true);

	CANTxManager_AddMessage(&Msg);
}

//-----------------------------------------------------------------------------
// Split and send Rx data in 8 byte messages
//-----------------------------------------------------------------------------
void CANTxManager_SendRxDataSequence(const uint8_t * pbData, uint32_t ulLength)
{
	while (ulLength > 0)
	{
		uint8_t bMessageLength = min(ulLength, 8);

		CANTxManager_SendRxData(pbData, bMessageLength);

		pbData += bMessageLength;
		ulLength -= bMessageLength;
	}
}

//-----------------------------------------------------------------------------
// Send the given data using Rx messages
// NOTES:
// The Rx message usually honour the flow control. This implementation will not.
// The max length is 255 and the GO's Rx buffer is larger than this.
// These messages are always handled in a full chunk and the buffer should be clear at the beginning.
//-----------------------------------------------------------------------------
void CANTxManager_SendWrappedRxDataPacket(uint8_t * pbData, uint8_t bLength)
{
	    // Beginning of data packet
		uint8_t bEndOfPacket = 0;
		CANTxManager_SendIoxInformationStatus(IOX_STATUS_PACKET_WRAPPER, &bEndOfPacket, sizeof(bEndOfPacket));

		CANTxManager_SendRxDataSequence(pbData, bLength);

		// End of data packet
		bEndOfPacket = 1;
		CANTxManager_SendIoxInformationStatus(IOX_STATUS_PACKET_WRAPPER, &bEndOfPacket, sizeof(bEndOfPacket));
}

// ****************************************************************
// *********************** STATIC METHODS *************************
// ****************************************************************

//-----------------------------------------------------------------------------
// Function   : CANTxManager_GetNextMessage
// Purpose    : Gets the next IOX message in the TX buffer
// Parameters : [O] Msg: Pointer to the extracted message
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
static void CANTxManager_GetNextMessage(IOExpanderMsg_t * Msg)
{
	memcpy(Msg, (uint8_t *) &mCANTx.MessageBuffer.Messages[mCANTx.MessageBuffer.bTail], sizeof(IOExpanderMsg_t));

	mCANTx.MessageBuffer.bTail++;
	mCANTx.MessageBuffer.bTail %= IOEXPANDER_MESSAGE_BUFFER_SIZE;	// Wraparound tail if neccessary

	mCANTx.MessageBuffer.bCount--;
}

// ****************************************************************
// ****************** START of CAN TX Manager SM ******************
// ****************************************************************

static void CANTxManager_SMIdle(bool fIsInit)
{
	mCANTx.Flags.Single.fBusy = false;

	if (mCANTx.MessageBuffer.bCount > 0)
	{
		IOExpanderMsg_t Msg;

		// Extract next message
		CANTxManager_GetNextMessage(&Msg);
		MB_ConvertToCANMsg(&mCANTx.MessageToSend, &Msg);

		if (Msg.fACKRequired)
		{
			mCANTx.Flags.Single.fWaitForACK = true;
			mCANTx.Flags.Single.fACKReceived = false;
			mCANTx.bACKType = Msg.bMsgType;
		}
		else
		{
			mCANTx.Flags.Single.fWaitForACK = false;
		}

		mCANTx.Flags.Single.fBusy = true;

		SM_SetNextState(CANTxManager_SMSendMessage);
	}
}

//-----------------------------------------------------------------------------
// Function   : CANTxManager_CheckMessageType
// Purpose    : Checks what type of message to be sent and
//				prints message type to user console
//
// Parameters : [I] pCANTxData, Transmit CAN msg
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
static void CANTxManager_CheckMessageType(CANTx_t *pCANTxData)
{
	uint32_t uiCanId = pCANTxData->MessageToSend.can_id;

	//Identify message type
	switch(((uiCanId & IO_EXPANDER_COMMAND_MASK) >>16))
	{
	case 0x02:
		printf("Sending message_id = 0x%x: Poll Response ACK\n", 0x02);
		break;
	case 0x03:
		printf("Sending message_id = 0x%x: ACK is received for the first poll response\n", 0x03);
		break;
	default:
		printf("Sending message_id = 0x%x: \n", (uint8_t)((uiCanId & IO_EXPANDER_COMMAND_MASK) >> 16));
		break;
	}
}

static void CANTxManager_SMSendMessage(bool fIsInit)
{
	//Check a message type to be sent
	CANTxManager_CheckMessageType(&mCANTx);

	CAN_SendMessage(&mCANTx.MessageToSend);

	if (mCANTx.Flags.Single.fWaitForACK)
	{
		SM_SetNextState(CANTxManager_SMWaitForACK);
	}
	else
	{
		SM_SetNextState(CANTxManager_SMIdle);	// No ACK needed
	}
}

static void CANTxManager_SMWaitForACK(bool fIsInit)
{
	if (fIsInit)
	{
		Timer_Reset(&mCANTx.Timer);
	}

	if (mCANTx.Flags.Single.fACKReceived)
	{
		mCANTx.Flags.Single.fWaitForACK = false;
		mCANTx.Flags.Single.fACKReceived = false;

		SM_SetNextState(CANTxManager_SMIdle);		// Message sent and acked successfully
	}
	else if (Timer_HasExpired(mCANTx.Timer, mCANTx.iACKTimeout))
	{
		printf("Timeout no ACK from GO\n");
		mCANTx.Flags.Single.fWaitForACK = false;
		mCANTx.Flags.Single.fACKReceived = false;
		SM_SetNextState(CANTxManager_SMIdle);
	}
}

// ****************************************************************
// ******************* END of CAN TX Manager SM *******************
// ****************************************************************

// ****************************************************************
// **************** START of CAN Tx Message Handlers **************
// ****************************************************************

void CANTxManager_AckReceived(IOExpanderMsg_t * Msg)
{
	printf("Received message_id = 0x%x: ACK from GO\n", MSG_ID_ACK);

	if (mCANTx.Flags.Single.fWaitForACK)
	{
		mCANTx.Flags.Single.fACKReceived = true;

		switch (mCANTx.bACKType)
		{
			case MSG_ID_POLL_RESPONSE:

				gfGoHandshakeAcked = true;

				break;

			case MSG_ID_RX_DATA:

				if ((Msg->bLength > 0) && (Msg->abData[0] & 0x01))	// Flow control bit is high
				{
					mCANTx.Flags.Single.fDelaySerialSending = true;
				}
				break;
		}
	}
}

// ****************************************************************
// ***************** END of CAN Tx Message Handlers ***************
// ****************************************************************

// End of file
