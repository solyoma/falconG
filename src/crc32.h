#pragma once

#include <QString>
#include <stdint.h>		// for uint32_t
#include <QByteArray>
#include <QString>
#include <QFileInfo>

//using string = QString;

class CRC32
{
	uint32_t  _ulCRC = 0;		// last CRC32 0: not initted

	static uint32_t _crc32_table[256];
	static bool _initted;

	uint32_t _Reflect(uint32_t ref, char ch);

	void _Init_CRC32_Table()
	{// Call this function only once to initialize the CRC table.
		if (_initted)
			return;

	 // This is the official polynomial used by CRC-32
	 // in PKZip, WinZip and Ethernet.
		uint32_t ulPolynomial = 0x04c11db7;

		// 256 values representing ASCII character codes.
		for (int i = 0; i <= 0xFF; i++)
		{
			_crc32_table[i] = _Reflect(i, 8) << 24;
			for (int j = 0; j < 8; j++)
				_crc32_table[i] = (_crc32_table[i] << 1) ^ (_crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
			_crc32_table[i] = _Reflect(_crc32_table[i], 32);
		}
		_initted = true;
	}

	uint32_t _checksum( QByteArray &buffer, bool reset=true)
	{
		if(reset)	// Start out with all bits set high.
			_ulCRC = 0xffffffff;	   // X.25 algorithm: using reflection and initial and  value final XOR with 0xFFFFFFFF;

		int len = buffer.size();
		// Perform the algorithm on each character
		// in the QString, using the lookup table values.
		for(int i =0; i < len; ++i)
			_ulCRC = (_ulCRC >> 8) ^ _crc32_table[(_ulCRC & 0xFF) ^ (unsigned char)buffer[i]];
		return _ulCRC;
	}

public:
	CRC32() { _Init_CRC32_Table();}
	CRC32(QString str) { QByteArray b; b.append(str); _Init_CRC32_Table(); crc(b); }
	uint32_t crc() const { return _ulCRC;  }

	uint32_t crc(QByteArray &text)
	{// Pass a text QString to this function and it will return the CRC.

	 // Once the lookup table has been filled in by the two functions above,
	 // this function creates all CRCs using only the lookup table.

	 // Be sure to use unsigned variables,
	 // because negative values introduce high bits
	 // where zero bits are required.

	 
		_ulCRC = _checksum(text);
		return _ulCRC ^= 0xffffffff;
	}

	uint32_t crc(QString fileName)
	{
		QFileInfo finfo(fileName);
		if (!finfo.exists())
			return 0;
		quint64 fsiz = finfo.size();
		QFile f(fileName);
		if (!f.open(QIODevice::ReadOnly))
			return 0;

		quint64 maxSize = 1024 * 1024;

		QByteArray buf;

		if (fsiz <= maxSize)
		{
			buf = f.readAll();
			return crc(buf);
		}
		else
		{
			buf = f.read(maxSize);
			_ulCRC = _checksum(buf);
			fsiz -= buf.size();

			do
			{
				buf = f.read(maxSize);
				if (buf.isEmpty())	// read error!
					return 0;
				_ulCRC = _checksum(buf, false);
				fsiz -= buf.size();
			} while (fsiz);
			return _ulCRC ^ 0xffffffff;
		}
	}
};