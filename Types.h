//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Platform built-in data types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User-defined data types
//-----------------------------------------------------------------------------

// Byte and bit access in unions
typedef union
{
	uint16_t iValue;
	struct
	{
		uint8_t b0;
		uint8_t b1;
	} Bytes;
} UInt16Bytes_t;

typedef union
{
	uint32_t iValue;
	struct
	{
		uint8_t b0;
		uint8_t b1;
		uint8_t b2;
		uint8_t b3;
	} Bytes;
} UInt32Bytes_t;

typedef union
{
	uint8_t bValue;
	struct
	{
		uint8_t b0 : 1;
		uint8_t b1 : 1;
		uint8_t b2 : 1;
		uint8_t b3 : 1;
		uint8_t b4 : 1;
		uint8_t b5 : 1;
		uint8_t b6 : 1;
		uint8_t b7 : 1;
	} Bits;
} ByteBits_t;

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define min(a,b)	(((a) < (b)) ? (a) : (b));

#endif // TYPES_H

// End of file

