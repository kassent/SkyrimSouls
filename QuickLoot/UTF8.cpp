
#include <SKSE.h>
#include <memory.h>		// for memcmp

enum { INVALID_CHAR = 0xffffffff };
enum { MAX_UTF8_BYTES = 6 };

struct EncTable
{
	int nbytes;
	unsigned long max;
};

static const EncTable table[] = {
	{ 1, 0x0000007F },
	{ 2, 0x000007FF },
	{ 3, 0x0000FFFF },
	{ 4, 0x001FFFFF },
	{ 5, 0x03FFFFFF },
	{ 6, 0x7FFFFFFF },
};


inline static int high_ones(int c) {
	int n;

	for (n = 0; (c & 0x80) == 0x80; c <<= 1)
		++n;
	return n;
}


static int EncodeUTF8(unsigned long u, unsigned char *buf)
{
	static const int ntab = sizeof(table) / sizeof(table[0]);
	int i, j;

	if (u > table[ntab - 1].max)
		return -1;

	for (i = 0; i < ntab; ++i) {
		if (u <= table[i].max)
			break;
	}

	if (table[i].nbytes > MAX_UTF8_BYTES)
		return -1;

	if (table[i].nbytes == 1) {
		buf[0] = u;
	}
	else {
		for (j = table[i].nbytes - 1; j > 0; --j) {
			buf[j] = 0x80 | (u & 0x3f);
			u >>= 6;
		}

		unsigned char mask = ~(0xFF >> table[i].nbytes);
		buf[0] = mask | u;
	}

	return table[i].nbytes;
}


static unsigned long DecodeUTF8(unsigned char *buf, int nbytes)
{

	unsigned long u;
	int i, j;

	if (nbytes <= 0)
		return INVALID_CHAR;

	if (nbytes == 1) {
		if (buf[0] >= 0x80)
			return INVALID_CHAR;
		return buf[0];
	}

	i = high_ones(buf[0]);
	if (i != nbytes)
		return INVALID_CHAR;
	u = buf[0] & (0xff >> i);
	for (j = 1; j < nbytes; ++j) {
		if ((buf[j] & 0xC0) != 0x80)
			return INVALID_CHAR;
		u = (u << 6) | (buf[j] & 0x3f);
	}

	if (u >= 0xD800 && u <= 0xDFFF)
		return INVALID_CHAR;
	if (u == 0xFFFE || u == 0xFFFF)
		return INVALID_CHAR;

	return u;
}


bool IsUTF8(const char *str) {
	unsigned char buf[MAX_UTF8_BYTES];
	unsigned char buf2[MAX_UTF8_BYTES];
	unsigned long code;
	int nbytes, nbytes2, c;

	nbytes = 0;

	do {
		c = *(unsigned const char *)str++;

		if (c == 0 || c < 0x80 || (c & 0xC0) != 0x80) {
			if (nbytes > 0) {
				code = DecodeUTF8(buf, nbytes);
				if (code == INVALID_CHAR)
					return false;
				nbytes2 = EncodeUTF8(code, buf2);
				if (nbytes != nbytes2 || memcmp(buf, buf2, nbytes) != 0)
					return false;
			}
			nbytes = 0;

			if (c != 0 && c >= 0x80)
				buf[nbytes++] = c;
		}
		else {
			if (nbytes == MAX_UTF8_BYTES)
				return false;
			buf[nbytes++] = c;
		}

	} while (c != 0);

	return (nbytes == 0);
}
