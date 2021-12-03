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
// Includes
//-----------------------------------------------------------------------------
#include "Types.h"
#include "IOExpander.h"
#include "ticktimer.h"
#include "CANTxManager.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define THIRD_PARTY_DATA_TIMEOUT_MS					3000
#define THIRD_PARTY_MODEM_STATUS_TIMEOUT_MS			15000		// We should receive a reply from the GO within 6s. If for some reason we don't, we will report a fail after this timeout.

#define ONE_K_BYTES									1024
#define MESSSAGE_BODY_LENGTH 						250
#define BINARY_DATA_SIZE 							ONE_K_BYTES*1

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------
typedef enum
{
	MODEM_RESULT_FAILED = 0,
	MODEM_RESULT_SUCCESS,
	MODEM_RESULT_UNKNOWN,
} tModemTransmissionResult;

typedef struct
{
	Timer_t Timer;
	tModemTransmissionResult TransmissionStatus;
} tThirdParty;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------
static uint8_t StartMimePassthrough;
static uint8_t BinaryData[BINARY_DATA_SIZE];
tThirdParty mThirdParty;
static uint32_t mulLength;

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------
static uint8_t* InitBinaryData(void);
static uint32_t GetBinaryDataSize(void);

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : InitBinaryData
// Purpose    : The function initializes binary data to be sent
// Parameters : None
// Return     : Pointer to the binary data
// Notes      : None
//-----------------------------------------------------------------------------
static uint8_t* InitBinaryData(void)
{
	uint32_t i;

	for (i = 0; i < BINARY_DATA_SIZE; i++)
	{
		BinaryData[i]= 0xAA;
	}

	return &BinaryData[0];
}

//-----------------------------------------------------------------------------
// Function   : GetBinaryDataSize
// Purpose    : The function returns the size of the binary data to be sent.
// Parameters : None
// Return     : Binary data size
// Notes      : None
//-----------------------------------------------------------------------------
static uint32_t GetBinaryDataSize(void)
{
	uint32_t BinaryDataLength;

	return	BinaryDataLength = BINARY_DATA_SIZE;
}

//-----------------------------------------------------------------------------
// Function   : ThirdParty_SetTransmissionStatus
// Purpose    : Used to set if the modem transmission succeeded or failed
// Parameters : fTransmissionResult
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void ThirdParty_SetTransmissionStatus(bool fTransmissionResult)
{
	mThirdParty.TransmissionStatus = (tModemTransmissionResult)fTransmissionResult;
}

//-----------------------------------------------------------------------------
// Function   : ThirdPartyInit
// Purpose    : This function updates a flag to start sending binary data to GO device.
// Parameters : *IOExpanderMsg
// Return     : None
// Notes      : This function is used for testing only and user should update the function
// 				to be align with the application.
//-----------------------------------------------------------------------------
void ThirdPartyInit(IOExpanderMsg_t * IOExpanderMsg)
{
	StartMimePassthrough = true;
}

//-----------------------------------------------------------------------------
// Function   : ThirdParty_ServiceComs
// Purpose    : This is  third party main handler to send binary frame packets
//              to the GO device. Maximum packet length is 250 bytes.
// Parameters : None
// Return     : None
// Notes      : This function is used for testing only and user should update the function
// 				to be align with the application.
//-----------------------------------------------------------------------------
void ThirdParty_ServiceComs(void)
{
	static enum
	{
		THIRD_PARTY_COM_INIT_STATE = 0,
		THIRD_PARTY_COM_IDLE_STATE,
		THIRD_PARTY_COM_SEND_DEVICE_CONNECTED,
		THIRD_PARTY_BINARY_DATA_PACKET_TO_SERVER,
		THIRD_PARTY_WAIT_FOR_MODEM_RESULT
	} eThirdPartyComState = THIRD_PARTY_COM_INIT_STATE;

	uint8_t static *pbData = NULL;

	switch (eThirdPartyComState)
	{
		case THIRD_PARTY_COM_INIT_STATE:

			StartMimePassthrough = false;
			mulLength = GetBinaryDataSize();
			pbData = InitBinaryData();
			eThirdPartyComState = THIRD_PARTY_COM_IDLE_STATE;
			break;

		case THIRD_PARTY_COM_IDLE_STATE:

			if (StartMimePassthrough == true)
			{
				eThirdPartyComState = THIRD_PARTY_COM_SEND_DEVICE_CONNECTED;
			}
			break;

		case THIRD_PARTY_COM_SEND_DEVICE_CONNECTED:

			CANTxManager_SendExternalDeviceConnected(EXTERNAL_DEVICE_TEST, 0, true);
			eThirdPartyComState = THIRD_PARTY_BINARY_DATA_PACKET_TO_SERVER;
			break;

		case THIRD_PARTY_BINARY_DATA_PACKET_TO_SERVER:

			if(mulLength > 0 )
			{
				uint8_t bMessageLength = min(mulLength, 250);

				CANTxManager_SendWrappedRxDataPacket(pbData, bMessageLength); // Sends 250 bytes to GO device

				pbData += bMessageLength;
				mulLength -= bMessageLength;

				mThirdParty.TransmissionStatus = MODEM_RESULT_UNKNOWN;
				Timer_Reset(&mThirdParty.Timer);
				eThirdPartyComState = THIRD_PARTY_WAIT_FOR_MODEM_RESULT;
			}
			else
			{
				eThirdPartyComState = THIRD_PARTY_COM_INIT_STATE;
			}
			break;

		case THIRD_PARTY_WAIT_FOR_MODEM_RESULT:

			if (mThirdParty.TransmissionStatus != MODEM_RESULT_UNKNOWN)	// Wait for the reply from the GO
			{
				uint32_t uiSuccess = (mThirdParty.TransmissionStatus == MODEM_RESULT_SUCCESS);
				printf("Modem result %d \n", uiSuccess);

				if(uiSuccess)
				{
					eThirdPartyComState = THIRD_PARTY_BINARY_DATA_PACKET_TO_SERVER;
				}
				else
				{
					eThirdPartyComState = THIRD_PARTY_COM_INIT_STATE;
				}
			}
			else if (Timer_HasExpired(mThirdParty.Timer, THIRD_PARTY_MODEM_STATUS_TIMEOUT_MS))
			{
				printf("Timeout, modem result hasn't been received from GO \n");
				eThirdPartyComState = THIRD_PARTY_COM_INIT_STATE;
			}
			break;

		default:
			break;
	}

}

// ****************************************************************
// ***************** END of Third Party Handlers ***************
// ****************************************************************

// End of file
