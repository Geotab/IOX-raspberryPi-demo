//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access
//or otherwise use this information.
//-----------------------------------------------------------------------------
// Module Notes:
// =============
//
// IOX to GO communication uses CAN bus
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "Types.h"
#include "string.h"
#include "IOExpander.h"
#include "statemachine.h"
#include "can.h"
#include "IOControl.h"
#include "InfoTable.h"
#include "MessageBuilder.h"
#include "CANTxManager.h"
#include "CANRxManager.h"
#include "NVM.h"
#include "MimePassthrough.h"
//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define CAN_RX_BUFFER_SIZE		16

// ACCF table sizes
#define CAN_EXTENDED_EXPLICIT_TABLE_SIZE	(sizeof(mCAN_EXTENDED_EXPLICIT_TABLE)/sizeof(mCAN_EXTENDED_EXPLICIT_TABLE[0]))

//CAN interface name
#define CAN_INTERFACE "can0"

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef struct
{
	uint8_t						bMsgType;
	IOExpanderMsgRxHandler_t	pfnCANRxHandler;
} CANExtendedExplicitHandler_t;

typedef struct
{
	int iSocket;

} CAN_t;

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------

static CAN_t mCAN;

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

static int CAN_ConfigureSocket(void);
static void CAN_CheckForMessageSM(bool fIsInit);
static void CAN_ConfigureACCF(uint16_t iSerialNumber);

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

// This table is used to configure the ACCF table. When a CAN msg is received it is passed on to the Handler which
// matches the Table ID from the ACCF.

static const CANExtendedExplicitHandler_t mCAN_EXTENDED_EXPLICIT_TABLE[] =
{
	//IOX Message Type						Message handler
	{MSG_ID_RESET,							IOControl_ResetHandler					},
	{MSG_ID_POLL,							IOControl_PollHandler					},
	{MSG_ID_WAKEUP,							IOControl_WakeupHandler					},
	{MSG_ID_SLEEP,							IOControl_SleepHandler					},
	{MSG_ID_TX_DATA,						IOControl_TxHandler						},
	{MSG_ID_ACK,							CANTxManager_AckReceived				},
	{MSG_ID_APPLICATION_SPECIFIC_DATA,		IOControl_ApplicationSpecificHandler	},
	{MSG_ID_GO_STATUS_INFORMATION,			InfoTable_StoreStatus					},
	{MSG_ID_GO_MULTIFRAME_DATA,				CANRxManager_MultiFrameMesssage			}
};

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : CAN_Init
// Purpose    : Initialise CAN
// Parameters : None
// Return     : The value returned 1 for success, 0 for failure
// Notes      : None
//-----------------------------------------------------------------------------
uint8_t CAN_Init(void)
{
	uint8_t fStatus = false;

	if (CAN_ConfigureSocket() == 0)
	{
		SM_Add(CAN_CheckForMessageSM);
		fStatus = true;
	}

	return fStatus;
}

//-----------------------------------------------------------------------------
// Function   : CAN_SendMessage
// Purpose    : Sends a message to GO device
// Parameters : [I] pCANMsg: The CAN msg to send
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void CAN_SendMessage(CANMsg_t* pCANMsg)
{
	write(mCAN.iSocket, pCANMsg, sizeof(CANMsg_t));
}

// ****************************************************************
// *********************** STATIC METHODS *************************
// ****************************************************************

//-----------------------------------------------------------------------------
// Function   : CAN_ConfigureSocket
// Purpose    : Configures CAN socket
// Parameters : None
// Return     : Status '0' for success '-1' or '-2' for failure opening socket
//				or error in socket bind respectively
// Notes      : None
//-----------------------------------------------------------------------------
static int CAN_ConfigureSocket(void)
{
	struct sockaddr_can addr;
	struct ifreq ifr;

	char *ifname = CAN_INTERFACE;

	//Create a socket
	if((mCAN.iSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
		return -1;	// Error while opening socket

	//Set receiving buffer size
	int rcvbuf_size =  10240;
	setsockopt(mCAN.iSocket, SOL_SOCKET, SO_RCVBUF,&rcvbuf_size, sizeof(rcvbuf_size));

	//Retrieve the interface index for the interface name (can0)
	strcpy(ifr.ifr_name, ifname);
	ioctl(mCAN.iSocket, SIOCGIFINDEX, &ifr);

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	CAN_ConfigureACCF(gIox.ulSerialNumber);

	//Bind the socket to the CAN Interface
	if(bind(mCAN.iSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return -2;	// Error in socket bind

	return 0;
}

//-----------------------------------------------------------------------------
// Function   : CAN_CheckForMessageSM
// Purpose    : Checks if a CAN message is in a FIFO and run the corresponding handler
//				Used for the external CAN
// Parameters : [I] fIsInit: Run initialisation if necessary
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
static void CAN_CheckForMessageSM(bool fIsInit)
{
	CANMsg_t CANMsg;

	int iBytesRead = recv(mCAN.iSocket, &CANMsg, sizeof(CANMsg), MSG_DONTWAIT);

	//Check if there are received bytes
	if (iBytesRead > 0)
	{
		unsigned int iIndex;

		for (iIndex = 0; iIndex < CAN_EXTENDED_EXPLICIT_TABLE_SIZE; iIndex++)
		{
			//Identify message type
			if (((CANMsg.can_id & CAN_EFF_MASK) >> 16) == mCAN_EXTENDED_EXPLICIT_TABLE[iIndex].bMsgType)
			{
				// It is neccessary to convert to IOX messages so that we have the ability to abstract the IOX
				// message from the CANMsg and always process the IOX data the same way
				IOExpanderMsg_t	IOExpanderRxMsg;
				MB_ConvertToIOExpanderMsg(&IOExpanderRxMsg, &CANMsg);
				mCAN_EXTENDED_EXPLICIT_TABLE[iIndex].pfnCANRxHandler(&IOExpanderRxMsg);
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Function   : CAN_ConfigureACCF
// Purpose    : Configure the ACCF using the values from the CAN message handler
//				table and the device S/N
// Parameters : [I] iSerialNumber: The 2 LSB's of the device S/N
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
static void CAN_ConfigureACCF(uint16_t iSerialNumber)
{
	struct can_filter rfilter[2 * CAN_EXTENDED_EXPLICIT_TABLE_SIZE];
	uint32_t lACCFEntry;
	unsigned int i;
	unsigned int iIndex = 0;

	for (i = 0; i < CAN_EXTENDED_EXPLICIT_TABLE_SIZE; i++)
	{
		lACCFEntry = CAN_EFF_FLAG;

		lACCFEntry |= (mCAN_EXTENDED_EXPLICIT_TABLE[i].bMsgType << 16);	// Add message Type

		rfilter[iIndex].can_id = lACCFEntry;							// Write broadcast entry
		rfilter[iIndex].can_mask = CAN_EFF_MASK;
		iIndex++;

		lACCFEntry |= iSerialNumber;									// Add S/N

		rfilter[iIndex].can_id = lACCFEntry;							// Write S/N entry
		rfilter[iIndex].can_mask = CAN_EFF_MASK;
		iIndex++;
	}

	setsockopt(mCAN.iSocket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
}

// End of file
