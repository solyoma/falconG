#include "crc32.h"

uint32_t CRC32::_crc32_table[256];
bool CRC32::_initted = false;

uint32_t CRC32::_Reflect(uint32_t ref, char ch)
{// Used only by Init_CRC32_Table().

	uint32_t value(0);

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for (int i = 1; i < (ch + 1); i++)
	{
		if (ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}
