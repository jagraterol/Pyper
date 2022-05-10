/* VSS $Header: /Viper/VPCmdIF/VPCmdIF/ViperInterface.c 1     9/03/19 6:30p Suzanne $
 */
/**
 *  @file ViperInterface.c
 *
 *  Contains C-language function implementations for Viper interface.
 */
#include "ViperInterface.h"
void crc16(uint32_t *crc, uint32_t data)
{
	static const char op[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
	data = (data ^ (*crc)) & 0xff;
	*crc >>= 8;

	if (op[data & 0xf] ^ op[data >> 4])
		*crc ^= 0xc001;

	data <<= 6;
	*crc ^= data;
	data <<= 1;
	*crc ^= data;

	return;
}

uint32_t Viper_CalcCRC_Bytes(uint8_t *data, uint32_t count)
{
	uint32_t crc;
	uint32_t n;

	crc = 0;
	for (n = 0; n < count; n++)
		crc16(&crc, data[n]);

	return crc;
}

// For Q15, the denominator is 2^15, or 32768
float FractToFloat(int16_t fract, float factor)
{
	return (float)(fract) / 32768 * factor;
}

float Deg_Fract2Float(int16_t i_deg) { return FractToFloat(i_deg, FFACTOR_EULER_DEGREE); }
float Rad_Fract2Float(int16_t i_rad) { return FractToFloat(i_rad, FFACTOR_EULER_RAD); }
float Qtrn_Fract2Float(int16_t i_qt) { return FractToFloat(i_qt, FFACTOR_QTRN); }
float Accel_Fract2Float(int16_t i_acc) { return FractToFloat(i_acc, FFACTOR_ACCEL); }

// fractional aVal /* fract value in range {-1, 1-2^-15} {-1, 0.999969482421875*/
// typedef int16_t fractional;
// float Fract2Float( fractional aVal )
//{
//
//#if    DATA_TYPE==FRACTIONAL        /* [ */
//
//	/* Local declarations. */
//	double scale = pow(2.0, -15.0);           /* 2^(-15) 0.00030517578125  = 1/32768*/
//	long int fullRange = 1L << 16;            /* 2^(16) 65536 0x10000*/
//	long int halfRange = 1L << 15;            /* 2^(15) 32768 0x08000*/
//	double decimalVal = 0.0;
//
//	/* Convert. */
//	if (aVal >= halfRange)
//	{
//		aVal -= fullRange;
//	}
//	decimalVal = ((double)aVal)*scale;
//
//	/* Return decimal value in floating point. */
//	return ((float)decimalVal);
//
//#else    /* ] [ */
//
//	/* Return input value in floating point. */
//	return ((float)aVal);
//
//#endif    /* ] */
//
//} /* end of Fract2Float */
//
// float AccelFract2Float(fractional i_accel)
//{
//	float f_accel = (float)(i_accel) / 32768.0 * 16.0;
//}
