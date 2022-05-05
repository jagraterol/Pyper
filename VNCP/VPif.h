// C++ wrappers for Viper Interface objects
/**
*  @file VPif.h
*
*  C++ implementation of ViperInterface.h structures and definitions.
*
*/

#pragma once
#include "VPcmdDefs.h"
#ifndef _WORKING
#include "ViperInterface.h"
#else
#include "ViperInterface-working.h"
#endif
#include <string>
#include <stdint.h>
#include <memory.h>
#include <algorithm>
#include <complex>

#ifndef CLAMP
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#endif

#ifndef FP_EQ
#define FP_EPSILON 0.00001
#define FP_EQ(x,v) (((v - FP_EPSILON) < x) && (x <( v + FP_EPSILON)))
#endif

#ifndef VPCMD_MAX
#define VPCMD_MAX CMD_MAX;
#endif

//typedef uint32_t VPCRC;
//// appears as the first 64 bytes of all frames to/from the device
//typedef struct _VP_FRAME_HDR
//{
//	uint32_t preamble;	///< 4 bytes that indicate what type of frame follows: 'VPRP' indicates a P&O frame, 'VPRC' indicates a command frame
//	uint32_t size;		///< 32-bit frame size indicates the number of bytes to follow, including terminating CRC.
//}VP_FRAME_HDR;

//                                     24            16                       32
#define MAX_VP_PNO_FRAME_SIZE (sizeof(SEUPNO_HDR) + (SENSORS_PER_SEU * sizeof(SENFRAMEDATA)) + sizeof(uint32_t)) //24+(16*32)+4 540 0x21C 
#define MIN_VP_PNO_FRAME_SIZE (sizeof(SEUPNO_HDR) + (0 *               sizeof(SENFRAMEDATA)) + sizeof(uint32_t)) //24+4 = 28 0x1C
//                                     28            4                       
#define MIN_VP_CMD_FRAME_SIZE (sizeof(SEUCMD_HDR) + sizeof(uint32_t))	// 28+4=32 0x20
#define MAX_VP_CMD_FRAME_SIZE UINT32_MAX // no max command frame size -- it is too variable
#define VP_NAK_FRAME_SIZE     (sizeof(SEUCMD_HDR) + sizeof(NAK_INFO) + sizeof(uint32_t)) //28+180+4=212 0xD4

#define VP_FRAME_PREAMBLE_SIZE (sizeof(uint32_t))
#define VP_FRAME_LEADER_SIZE   (sizeof(VP_FRAME_HDR))  //Size of preamble + size. Same for P&O and Cmd frames 
#define VP_FRAME_CRC_SIZE      (sizeof(uint32_t))      //Size of CRC field at end of all frames.

#define VP_PNO_FRAME_SIZE_FIELD_MIN	(MIN_VP_PNO_FRAME_SIZE - VP_FRAME_LEADER_SIZE) //28-8=20 0x14
#define VP_PNO_FRAME_SIZE_FIELD_MAX	(MAX_VP_PNO_FRAME_SIZE - VP_FRAME_LEADER_SIZE) //124-8=116 0x74

#define VP_CMD_FRAME_SIZE_FIELD_MIN	(MIN_VP_CMD_FRAME_SIZE - VP_FRAME_LEADER_SIZE)
#define VP_CMD_FRAME_SIZE_FIELD_MAX	(MAX_VP_CMD_FRAME_SIZE - VP_FRAME_LEADER_SIZE)

#define RPT_CRLF "\r\n"
#define RPT_INDENT "   "
extern char *RPT_szCRLF;// = RPT_CRLF;
extern char *RPT_szINDENT;// = "";


//! A small-footprint object used to pass around references to complete dev I/O frames
///   without copying and re-copying the frame buffer.  When a frame is received by the host, it is
///   read directly into a buffer owned by CVPdevIO.  From there it is not copied
///   again until the user app retrieves it.   Until that time it is referenced by an instance of
///   CFrameInfo that includes the following information:
///   - A pointer to the beginning of the frame in the CVPdevIO buffer.
///   - The total number of bytes in the frame.  (This is not the *size* field of the frame; it is the
///      count of bytes that came from the IO object from the first byte of the VP_FRAME_HDR to the last
///      of the CRC at the end.
typedef struct _vpFrameInfo
{
	uint8_t * pF;			///< Pointer to buffer location where raw frame bytes are stored 
	uint32_t uiSize;		///< Size of the P&O frame in bytes. Size of the entire buffer.
	uint32_t uiFCountRx;	///< Count of frames received from this dev since the dev was discovered. 
	int32_t iFrameErr;	    ///< Error status of the frame.
	uint64_t ts;			///< Optional timestamp: time since system clock epoch
} vpFrameInfo;

//! C++ class implentation of @ref vpFrameInfo struct.
/// A CFrameInfo object is a small-footprint object used to pass around complete dev I/O frames
///   without copying and re-copying the frame buffer.  When a frame is received by the host, it is
///   read directly into a buffer owned by CVPdevIO.  From there it is not copied
///   again until the user app retrieves it.   Until that time it is referenced by an instance of
///   CFrameInfo that includes the following information:
///   - A pointer to the beginning of the frame in the CVPdevIO buffer.
///   - The total number of bytes in the frame.  (This is not the *size* field of the frame; it is the
///      count of bytes that came from the IO object from the first byte of the VP_FRAME_HDR to the last
///      of the CRC at the end.
class VPCMD_API CFrameInfo : public vpFrameInfo
{
public:
	CFrameInfo() { Init(); }
	
	CFrameInfo(uint8_t* p, uint32_t size, uint32_t FC, uint32_t FE)
	{
		Init();
		pF = p; uiSize = size;
		uiFCountRx = FC; 
		iFrameErr = FE;
	}
	
	CFrameInfo(uint8_t* p, uint32_t size, uint32_t fc=0, int32_t FE=0, uint64_t ats=0)
	{
		Init();
		pF = p; uiSize = size; uiFCountRx = fc; iFrameErr = FE; ts = ats;
	}
	
	//CFrameInfo(const CFrameInfo & rv)
	//{
	//	pF = rv.pF;
	//	uiSize = rv.uiSize;
	//	uiFCountRx = rv.uiFCountRx;
	//	iFrameErr = rv.iFrameErr;
	//  ts = rv.ts;
	//}

	CFrameInfo(vpFrameInfo & rv)
	{
		memcpy(&pF, &rv.pF, sizeof(vpFrameInfo));
	}

	CFrameInfo & operator=(const CFrameInfo & rv)
	{
		pF = rv.pF;
		uiSize = rv.uiSize;
		uiFCountRx = rv.uiFCountRx;
		iFrameErr = rv.iFrameErr;
		ts = rv.ts;

		return *this;
	}

	void DeepCopy(uint8_t *pbuf, uint32_t uiBufsize, const CFrameInfo & rv)
	{
		uiSize = std::min<uint32_t>(uiBufsize, rv.uiSize);
		uiFCountRx = rv.uiFCountRx;
		iFrameErr = rv.iFrameErr;
		ts = rv.ts;

		if (pbuf)
		{
			pF = pbuf;
			memcpy(pF, rv.pF, uiSize);
		}
	}

	void Init()
	{
		pF = 0; uiSize = 0;
		uiFCountRx = 0;  
		iFrameErr = 0;
		ts = 0;
	}
	
	uint32_t cmd() const
	{
		if (IsCmd())
			return ((SEUCMD_HDR*)pF)->seucmd.cmd;
		else
			return (uint32_t)-1;
	}
	
	uint32_t action() const 
	{
		if (IsCmd())
			return ((SEUCMD_HDR*)pF)->seucmd.action;
		else
			return (uint32_t)-1;
	}
	
	uint32_t devid() const
	{
		if (IsCmd() || IsPno())
			return ((SEUCMD_HDR*)pF)->seucmd.seuid;
		else
			return (uint32_t)-1;
	}

	uint32_t devfc() const
	{
		if (IsNull() || IsCmd())
			return 0;
		else
			return ((SEUPNO_HDR*)pF)->seupno.frame;
	}

	int32_t err() const
	{
		return iFrameErr;
	}

	uint64_t & TS() 
	{
		return ts;
	}

	bool IsCmd() const
	{
		if (IsNull())
			return false;
		else if (((VP_FRAME_HDR*)pF)->preamble == VIPER_CMD_PREAMBLE)
			return true;
		else
			return false;
	}
	//{
	//	if (IsNull())
	//		return false;
	//	else
	//		return (((LPVP_FRAME_HDR)pF)->preamble == VIPER_CMD_PREAMBLE);
	//}

	bool IsPno() const
	{
		if (IsNull())
			return false;
		else if (((VP_FRAME_HDR*)pF)->preamble == VIPER_PNO_PREAMBLE)
			return true;
		else
			return false;
	}

	uint32_t Preamble() const
	{
		return ((VP_FRAME_HDR*)pF)->preamble;
	}


	bool IsNull() const
	{
		return (pF == 0);
	}

	bool IsNak() const
	{
		if (IsCmd())
			return (action() == CMD_ACTION_NAK);

		else
			return false;
	}

	bool IsNakWarning() const
	{
		if (IsCmd())
			return (action() == CMD_ACTION_NAK_WARNING);

		else
			return false;
	}

	bool IsAck() const
	{
		if (IsCmd())
			return (action() == CMD_ACTION_ACK);

		else
			return false;
	}


	uint8_t * PCmdPayload()
	{
		if (IsCmd() && !IsNull())
			return &pF[sizeof(SEUCMD_HDR)];
		else
			return 0;
	}
	
	uint32_t CmdPayloadSize()
	{
		if (IsCmd() && !IsNull())
		{
			return ((SEUCMD_HDR*)pF)->size - sizeof(SEUCMD) - VP_FRAME_CRC_SIZE;
		}
		else
			return 0;
	}

	uint8_t *PPnoBody()
	{
		if (IsPno() && !IsNull())
			return &pF[sizeof(VP_FRAME_HDR)];
		else
			return 0;
	}
	SENFRAMEDATA *pSen(int32_t s = 0)
	{
		if (IsPno() && !IsNull())
			return (SENFRAMEDATA *)&pF[sizeof(SEUPNO_HDR) + (s * sizeof(SENFRAMEDATA))];
		else
			return 0;
	}
	uint32_t PnoBodySize()
	{
		if (IsPno() && !IsNull())
		{
			return ((SEUPNO_HDR*)pF)->size - VP_FRAME_CRC_SIZE;
		}
		else
			return 0;
	}
};

class VPCMD_API CVPcmd : public SEUCMD_HDR
{
public:
	CVPcmd( uint32_t pre = VIPER_CMD_PREAMBLE ) : ppay(0), szpay(0)
	{
		Init(pre);
	}

	CVPcmd(const CVPcmd & rv)
	{
		*this = rv;
	}

	void *       operator () (CVPcmd & rv)       { return (void*)&rv.preamble; }
	const void * operator () (const CVPcmd & rv) { return (const void*)&rv.preamble; }

	CVPcmd & operator = (const CVPcmd & rv)
	{
		memcpy(this, (const void *)&rv.preamble, sizeof(SEUCMD_HDR));
		return *this;
	}

	operator SEUCMD_HDR * () { return (SEUCMD_HDR *)this; }

	void Init( uint32_t pre=VIPER_CMD_PREAMBLE )
	{
		ppay = 0; szpay = 0;
		preamble = pre; // VIPER_CMD_PREAMBLE;
		size = sizeof(SEUCMD) + sizeof(VPCRC);
	}

	void Fill(uint32_t id, uint32_t cmd, uint32_t act, uint32_t a1 = 0, uint32_t a2 = 0, void *pp=0, uint32_t szp=0)
	{
		seucmd.seuid = id;
		seucmd.cmd = cmd;
		seucmd.action = act;
		seucmd.arg1 = a1;
		seucmd.arg2 = a2;
		if (act == CMD_ACTION_SET)
		{
			ppay = pp;
			szpay = szp;
			size += szp;
		}
		else
		{
			ppay = 0;
			szpay = 0;
		}
	}

	uint32_t & Seu() {
		return seucmd.seuid;
	}
	uint32_t & Cmd() {
		return seucmd.cmd;
	}
	uint32_t & Act() {
		return seucmd.action;
	}
	uint32_t & SnsArg() {
		return Arg1();
	}
	uint32_t & Arg1() {
		return seucmd.arg1;
	}
	uint32_t & Arg2() {
		return seucmd.arg2;
	}

	eViperCmds  ECmd() { return (eViperCmds)seucmd.cmd; }

	eCmdActions  EAct() { return (eCmdActions)seucmd.action; }

	bool IsGet() { return EAct() == CMD_ACTION_GET; }
	bool IsSet() { return EAct() == CMD_ACTION_SET; }
	bool IsReset() { return EAct() == CMD_ACTION_RESET; }
	bool IsAck() { return EAct() == CMD_ACTION_ACK; }
	bool IsNak() { return EAct() == CMD_ACTION_NAK; }
	bool IsNakWarning() { return EAct() == CMD_ACTION_NAK_WARNING; }

	void Prepare( uint8_t buf[], int & txbytes)
	{
		CVPcmd *ptx = (CVPcmd*)buf;

		//*ptx = *this;
		uint32_t crc_count = sizeof(SEUCMD_HDR);
		memcpy(ptx, (const void *)this, sizeof(SEUCMD_HDR));
		if (ppay)
		{
			memcpy(&buf[sizeof(SEUCMD_HDR)], ppay, szpay);
			crc_count += szpay;
		}

		uint32_t *pcrc = (uint32_t*)&buf[crc_count];
		*pcrc = CalcCRC_Bytes(buf, crc_count);

		txbytes = crc_count + sizeof(uint32_t);
	}

	static size_t paysize(eViperCmds cmd);
	static size_t framesize(eViperCmds cmd);
	static const char *cmdstr(uint32_t cmd);
	static const char *actstr(uint32_t act);
	static const char *posunitsstr(uint32_t posu, bool bpub=false, bool bbrief=false);
	static const char *oriunitsstr(uint32_t oriu, bool bpub = false, bool bbrief = false);
	static const char *frameratestr(uint32_t frate);
	static const char *filterlevstr(uint32_t flev);
	static const char *filtertarstr(uint32_t ftar);
	static const char *fttstr(uint32_t ftt);
	static const char *hemispherestr(uint32_t hemval);
	static const char *vphemispherestr(uint32_t hemval);
	static const char *baudstr(uint32_t b);
	static const char *paritystr(uint32_t p);
	static const char *baudvalstr(uint32_t b); // valstr() fcns return human parsable text strings
	static const char *parityvalstr(uint32_t p);
	static const char * bitcodestr(uint32_t p);
	static const char * bitcodevalstr(uint32_t p);
	static const char *stylusmodestr(uint32_t p);
	static const char *snsorigstr(uint32_t p);
	static const HEMISPHERE_CONFIG *hemispherecfg(uint32_t hemval);

	static const int baudbps(uint32_t b);
	static const eBaud bpsbaud(int32_t bps);

	static void InitCmdInfo();
	static void InitActInfo();
	static void InitUnitsInfo();
	static void InitFRateInfo();
	static void InitFilterLevInfo();
	static void InitFilterTarInfo();
	static void InitFTTInfo();
	static void InitHemisphereInfo(); //VPif-only enum for CHemisphereCfg
	static void InitViperHemisphereInfo();
	static void InitBaudInfo();
	static void InitParityInfo();
	static void InitBITcodeInfo();
	static void InitStylusInfo();
	static void InitSnsOrigInfo();

	static bool ValidateLeader(uint8_t * pbuf, uint32_t cmdPre = VIPER_CMD_PREAMBLE, uint32_t pnoPre = VIPER_PNO_PREAMBLE);

	static bool ValidateCRC(uint8_t *pbuf)
	{
		uint32_t size = (pbuf == 0) ? 0 : ((VP_FRAME_HDR*)pbuf)->size;

		if (size == 0)
			return false;

		uint32_t count = 0;
		uint32_t *pCrc = 0;
		uint32_t crcCalc = 0;
		uint32_t crcFrame = 0;

		count = VP_FRAME_LEADER_SIZE + size - VP_FRAME_CRC_SIZE;
		pCrc = (uint32_t*)(pbuf + VP_FRAME_LEADER_SIZE + size - VP_FRAME_CRC_SIZE);

		crcFrame = *pCrc;
		crcCalc = CalcCRC_Bytes(pbuf, count);

		return (crcFrame == crcCalc);
	}

	static bool ValidateAct(eViperCmds, eCmdActions);
	
	static void crc16(uint32_t * crc, uint32_t data)
	{
		static const char op[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
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

	static uint32_t CalcCRC_Bytes(uint8_t *data, uint32_t count)
	{
		uint32_t crc;
		uint32_t n;

		crc = 0;
		for (n = 0; n<count; n++)
			crc16(&crc, data[n]);

		return crc;
	}
	// For Q15, the denominator is 2^15, or 32768
	static float FractToFloat(int16_t fract, float factor)
	{
		return (float)(fract) / 32768 * factor;
	}

	static eViperCmds tag2vpcmd(uint32_t); //place holder
	static uint32_t   vp2tagcmd(eViperCmds); //place holder
	void   vp2tag();
	static uint32_t TrkCmdPre(uint32_t pid);
	static uint32_t TrkPnoPre(uint32_t pid);

	void * ppay;
	uint32_t szpay;

};

typedef struct _VP_PNO
{
	VP_FRAME_HDR hdr;
	SEUPNO seupno;
	SENFRAMEDATA sarr[SENSORS_PER_SEU];
}VP_PNO;

typedef struct _SNSFRMALL
{
	SENFRAMEDATA f;
	float accel[4];
}SNSFRMALL;
class VPCMD_API CVPSnsFrameA : public SNSFRMALL
{
public:
	//CVPSnsFrameA() 
	//{
	//	Init();
	//}
	//CVPSnsFrameA(SENFRAMEDATA *pF=0)
	//{
	//	Fill(pF);
	//}
	CVPSnsFrameA(SENFRAMEDATA_A *pFA=0)
	{
		Fill(pFA);
	}
	operator SENFRAMEDATA * () { return &f; }

	void Init()
	{
		memset(&f, 0, sizeof(SNSFRMALL));
	}
	bool Qtrn()
	{
		return f.SFinfo.bfOriUnits == ORI_QUATERNION;
	}
	int NumTerms()
	{
		return Qtrn() ? 4 : 3;
	}
	float OriFactor()
	{
		return OriFractFactor[f.SFinfo.bfOriUnits];
	}
	void ConvertOri(int16_t *pFracts, int count, float factor)
	{
		for (int i = 0; i < count; i++)
		{
			f.pno.ori[i] = CVPcmd::FractToFloat(pFracts[i], factor);
		}
	}
	void ConvertAccel(int16_t *pFracts, int count=4, float factor=FFACTOR_ACCEL)
	{
		for (int i = 0; i < count; i++)
		{
			accel[i] = CVPcmd::FractToFloat(pFracts[i], factor);
		}
	}
	void Fill(SENFRAMEDATA_A *pFA=0)
	{
		if (!pFA)
		{
			Init();
			return;
		}
		f.SFinfo = pFA->SFinfo;
		memcpy(f.pno.pos, pFA->pno.pos, sizeof(f.pno.pos));
		ConvertOri(pFA->pno.ori, Qtrn() ? 4 : 3, OriFractFactor[f.SFinfo.bfOriUnits]);
		ConvertAccel(pFA->pno.acc);

	}
	void Fill(SENFRAMEDATA *pF = 0)
	{
		if (!pF)
		{
			Init(); 
			return;
		}
		memcpy(&f, pF, sizeof(SENFRAMEDATA));
	}

	static float OriFractFactor[ORI_MAX];

};


class VPCMD_API CVPSeuPno : public VP_PNO
{
public:
	CVPSeuPno()
	{
		Init();
	}

	CVPSeuPno(const CVPSeuPno & rv)
	{
		memcpy(&hdr, &rv.hdr, sizeof(VP_PNO));
	}

	CVPSeuPno(const VP_PNO * prv)
	{
		Init();
		memcpy(&hdr, prv, sizeof(VP_PNO));
	}

	CVPSeuPno(uint8_t *p)
	{
		Init();
		memcpy(&hdr, p, sizeof(VP_PNO));
	}

	operator VP_PNO * () { return (VP_PNO *)this; }

	CVPSeuPno & operator= (const CVPSeuPno & rv)
	{
		memcpy(&hdr, &rv.hdr, sizeof(VP_PNO));
		return *this;
	}

	CVPSeuPno & operator= (const VP_PNO * prv)
	{
		memcpy(&hdr, prv, sizeof(VP_PNO));
		return *this;
	}

	CVPSeuPno & operator= (uint8_t *p)
	{
		memcpy(&hdr, p, sizeof(VP_PNO));
		return *this;
	}

	uint32_t Extractraw(uint8_t *p)
	{
		if (!p)
			return 0;

		VP_FRAME_HDR *ph = (VP_FRAME_HDR*)p;
		if (ph->preamble != VIPER_PNO_PREAMBLE)
			return 0;

		Init();
		uint32_t index = 0;
		memcpy(&hdr, ph, sizeof(VP_FRAME_HDR)); index += sizeof(VP_FRAME_HDR);
		memcpy(&seupno, &p[index], sizeof(SEUPNO)); index += sizeof(SEUPNO);
		for (uint32_t i=0; i < seupno.sensorCount; i++)
		{
			memcpy(&sarr[i], &p[index], sizeof(SENFRAMEDATA)); index += sizeof(SENFRAMEDATA);
		}

		index += sizeof(VPCRC);
		return index;
	}

	uint32_t Extractseupno(uint8_t *p)
	{
		if (!p)
			return 0;

		//VP_FRAME_HDR *ph = (VP_FRAME_HDR*)p;
		//if (ph->preamble != VIPER_PNO_PREAMBLE)
		//	return 0;

		Init();
		uint32_t index = 0;
		//memcpy(&hdr, ph, sizeof(VP_FRAME_HDR)); index += sizeof(VP_FRAME_HDR);
		memcpy(&seupno, &p[index], sizeof(SEUPNO)); index += sizeof(SEUPNO);
		for (uint32_t i = 0; i < seupno.sensorCount; i++)
		{
			memcpy(&sarr[i], &p[index], sizeof(SENFRAMEDATA)); index += sizeof(SENFRAMEDATA);
		}

		index += sizeof(VPCRC);
		return index;
	}

	uint32_t SensorCount() const { return seupno.sensorCount; }

	uint32_t SensorMap() const
	{
		uint32_t map = 0;

		for (uint32_t i = 0; i < seupno.sensorCount; i++)
		{
			const SENFRAMEDATA*  pSD = &(sarr[i]);

			map |= (1 << pSD->SFinfo.bfSnum);
		}
		return map;
	}

	SENFRAMEDATA * SensFrame(int i)
	{
		if (i < (int)SensorCount())
			return &(sarr[i]);
		else
			return 0;
	}

	SENFRAMEDATA_A * SensFrameA(int i)
	{
		if (i < (int)SensorCount())
			return (SENFRAMEDATA_A*)(&(sarr[i]));
		else
			return 0;
	}

	uint32_t Mode()
	{
		return seupno.HPinfo.bfPnoMode;
	}

	uint32_t BITerr()
	{
		return seupno.HPinfo.bfBITerr;
	}

	void Init()
	{
		memset(&hdr, 0, sizeof(VP_PNO));
	}

	void MakePrintable()
	{

	}

	void ReportHdr(std::string & s, bool bOneline = false, bool bZerobased = true, bool bAccel=false)
	{
		MakePrintable();

		uint32_t	nSeuID = seupno.seuid;// hubpno.hubid;
		uint32_t	nFrameNum = seupno.frame;
		uint32_t	nSCount = seupno.sensorCount;
		uint32_t	nPnoMode = seupno.HPinfo.bfPnoMode;
		int32_t     nAdd = (bZerobased) ? 0 : 1;

		//i += sizeof(SEUPNO_HDR);

		char szData[500];
		CVPSnsFrameA sd_a;

		if (bOneline)
		{
			//seuid + framename

			sprintf_s(szData, _countof(szData), "%2d  |  %07d  ", nSeuID+nAdd, nFrameNum);
			s += szData;

			for (uint32_t j = 0; j < nSCount; j++)
			{
				SENFRAMEDATA*  pSD = &(sarr[j]);
				float *pAccel = 0;
				char b0, b1;
				b0 = '0' + (pSD->SFinfo.bfBtnState0);
				b1 = '0' + (pSD->SFinfo.bfBtnState1);
				uint8_t dist = pSD->SFinfo.bfDistortion;
				uint16_t aux = pSD->SFinfo.bfAuxInput;
				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}
				
				//sensor num
				sprintf_s(szData, _countof(szData), "|  %2d  ",//  %c%c  %03d  %04x  |",
					pSD->SFinfo.bfSnum + nAdd);
				s += szData;

				//bitfields:stylus
				sprintf_s(szData, _countof(szData), "%c%c  ",
					b0, b1);
				s += szData;
				//bitfields:distortion
				sprintf_s(szData, _countof(szData), "%03d  ",
					dist);
				s += szData;
				//bitfields:aux
				sprintf_s(szData, _countof(szData), "%04x  ",
					aux);
				s += szData;

				//position
#ifndef _EP
				sprintf_s(szData, _countof(szData), "|  % 7.3f % 7.3f % 7.3f   ",
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
#else
				sprintf_s(szData, _countof(szData), "|  % 7.5f % 7.5f % 7.5f   ",
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
#endif
				s += szData;

				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
#ifndef _EP
					sprintf_s(szData, _countof(szData), "|  % 8.4f % 8.4f % 8.4f % 8.4f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
#else
					sprintf_s(szData, _countof(szData), "|  % 8.6f % 8.6f % 8.6f % 8.6f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
#endif
				}
				else
				{
#ifndef _EP
					sprintf_s(szData, _countof(szData), "|  % 8.3f % 8.3f % 8.3f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
#else
					sprintf_s(szData, _countof(szData), "|  % 8.5f % 8.5f % 8.5f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
#endif
				}
				s += szData;

				if (bAccel)
				{
					if (!pAccel)
					{
						sprintf_s(szData, _countof(szData),
							" No_Accel \r\n");
					}
					else
					{
#ifndef _EP
						sprintf_s(szData, _countof(szData),
							" % 8.3f  % 8.3f  % 8.3f  % 8.3f \r\n"
							,pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
#else
						sprintf_s(szData, _countof(szData),
							" % 8.5f  % 8.5f  % 8.5f  % 8.5f \r\n"
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
#endif
					}
					s += szData;
				}
			}
		}
		else //multi-line
		{
			RPT_szCRLF = bAccel ? "" : RPT_CRLF;
			//for (uint32_t j = 0; j < nSCount; j++) 
			{
				SENFRAMEDATA*  pSD = &(sarr[0]);
				float *pAccel = 0;
				char b0, b1;
				b0 = '0' + (pSD->SFinfo.bfBtnState0);
				b1 = '0' + (pSD->SFinfo.bfBtnState1);
				uint8_t dist = pSD->SFinfo.bfDistortion;
				uint16_t aux = pSD->SFinfo.bfAuxInput;
				const char * szPosu = CVPcmd::posunitsstr(pSD->SFinfo.bfPosUnits, false, true);
				const char * szOriu = CVPcmd::oriunitsstr(pSD->SFinfo.bfOriUnits, false, true);

				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}

				//seu sens frame
				sprintf_s(szData, _countof(szData),
					//"%2d  %2d  %09d  ",
				  //"12  12  123456789  ",
					"SEU Sns  FrameNum  ");
				s += szData;


				// bf: stylus
				sprintf_s(szData, _countof(szData),
					"BB  ");
				s += szData;

				//pos
#ifndef _EP
				sprintf_s(szData, _countof(szData),
					//"  %- 9.3f %- 9.3f %- 9.3f ");
					"  X(%s).... Y(%s).... Z(%s).... ", szPosu, szPosu, szPosu);
#else
				sprintf_s(szData, _countof(szData),
					//"  %- 9.5f %- 9.5f %- 9.5f ",
					"  X(%s).... Y(%s).... Z(%s).... ", szPosu, szPosu, szPosu);
#endif
				s += szData;
#ifndef _EP
				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData),
						//" %- 8.4f %- 8.4f %- 8.4f %- 8.4f %s",
						  " Qw...... Qx...... Qy...... Qz...... ");
				}
				else
				{
					sprintf_s(szData, _countof(szData),
						//" %- 8.3f %- 8.3f %- 8.3f %s",
						" Az(%3s). El(%3s). Ro(%3s). ", szOriu, szOriu, szOriu);
				}
#else
				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData),
						//" %- 8.6f %- 8.6f %- 8.6f %- 8.6f %s",
						" Qw...... Qx...... Qy...... Qz...... ");
				}
				else
				{
					sprintf_s(szData, _countof(szData),
						//" %- 8.5f %- 8.5f %- 8.5f %s",
						" Az(%3s). El(%3s). Ro(%3s). ", szOriu, szOriu, szOriu);
			}

#endif
				s += szData;

				if (bAccel && pAccel)
				{
#ifndef _EP
					sprintf_s(szData, _countof(szData),
						//" % 8.3f  % 8.3f  % 8.3f  % 8.3f \r\n"
						//"Aa(X)5678AA12345678AA12345678AA12345678A");
						  " a(X)....  a(Y)....  a(Z)....  a(R).... ");

#else
					sprintf_s(szData, _countof(szData),
						//" % 8.5f  % 8.5f  % 8.5f  % 8.5f \r\n"
						" a(X)....  a(Y)....  a(Z)....  a(R).... ");
#endif
					s += szData;
				}

			}
		}

	}
	void Report(std::string & s, bool bOneline = false, bool bZerobased = true, bool bAccel = false)
	{
		MakePrintable();

		uint32_t	nSeuID = seupno.seuid;// hubpno.hubid;
		uint32_t	nFrameNum = seupno.frame;
		uint32_t	nSCount = seupno.sensorCount;
		uint32_t	nPnoMode = seupno.HPinfo.bfPnoMode;
		int32_t     nAdd = (bZerobased) ? 0 : 1;

		//i += sizeof(SEUPNO_HDR);

		char szData[500];
		CVPSnsFrameA sd_a;

		if (bOneline)
		{
			//seuid + framename

			//sprintf_s(szData, _countof(szData), "%3d |  %07d |  0x%04x ", nSeuID, nFrameNum, nDigIO);
			sprintf_s(szData, _countof(szData), "%2d  |  %07d  ", nSeuID + nAdd, nFrameNum);
			s += szData;

			for (uint32_t j = 0; j < nSCount; j++)
			{
				SENFRAMEDATA*  pSD = &(sarr[j]);
				float *pAccel = 0;
				char b0, b1;
				b0 = '0' + (pSD->SFinfo.bfBtnState0);
				b1 = '0' + (pSD->SFinfo.bfBtnState1);
				uint8_t dist = pSD->SFinfo.bfDistortion;
				uint16_t aux = pSD->SFinfo.bfAuxInput;
				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}

				//sensor num + bitfields + position
				//sprintf_s(szData, _countof(szData), "|  %2d  %c%c  %03d  %04x  |  % 7.3f % 7.3f % 7.3f   |",
				//	pSD->SFinfo.bfSnum + nAdd, b0, b1, dist, aux, pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
				//s += szData;

				//sensor num + all bitfields
				//sprintf_s(szData, _countof(szData), "|  %2d  %c%c  %03d  %04x  |",
				//	pSD->SFinfo.bfSnum + nAdd, b0, b1, dist, aux);
				//s += szData;

				//sensor num
				sprintf_s(szData, _countof(szData), "|  %2d  ",//  %c%c  %03d  %04x  |",
					pSD->SFinfo.bfSnum + nAdd);
				s += szData;

				//all bitfields
				//sprintf_s(szData, _countof(szData), "  %c%c  %03d  %04x  |",
				//	b0, b1, dist, aux);
				//s += szData;

				//bitfields:stylus
				sprintf_s(szData, _countof(szData), "%c%c  ",
					b0, b1);
				s += szData;
				//bitfields:distortion
				sprintf_s(szData, _countof(szData), "%03d  ",
					dist);
				s += szData;
				//bitfields:aux
				sprintf_s(szData, _countof(szData), "%04x  ",
					aux);
				s += szData;

				//position
#ifndef _EP
				sprintf_s(szData, _countof(szData), "|  % 7.3f % 7.3f % 7.3f   ",
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
#else
				sprintf_s(szData, _countof(szData), "|  % 7.5f % 7.5f % 7.5f   ",
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
#endif
				s += szData;

				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					//sprintf_s(szData, _countof(szData), "| %3d  % 7.3f % 7.3f % 7.3f | % 8.4f % 8.4f % 8.4f % 8.4f ",
					//	pSD->SFinfo.bfSnum, pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2],
#ifndef _EP
					sprintf_s(szData, _countof(szData), "|  % 8.4f % 8.4f % 8.4f % 8.4f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
#else
					sprintf_s(szData, _countof(szData), "|  % 8.6f % 8.6f % 8.6f % 8.6f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
#endif
				}
				else
				{
					//sprintf_s(szData, _countof(szData), "| %3d  % 7.3f % 7.3f % 7.3f | % 8.3f % 8.3f % 8.3f ",
					//	pSD->SFinfo.bfSnum, pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2],
#ifndef _EP
					sprintf_s(szData, _countof(szData), "|  % 8.3f % 8.3f % 8.3f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
#else
					sprintf_s(szData, _countof(szData), "|  % 8.5f % 8.5f % 8.5f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
#endif
				}
				s += szData;

				if (bAccel)
				{
					if (!pAccel)
					{
						sprintf_s(szData, _countof(szData),
							" No_Accel \r\n");
					}
					else
					{
#ifndef _EP
						sprintf_s(szData, _countof(szData),
							" % 8.3f  % 8.3f  % 8.3f  % 8.3f \r\n"
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
#else
						sprintf_s(szData, _countof(szData),
							" % 8.5f  % 8.5f  % 8.5f  % 8.5f \r\n"
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
#endif
					}
					s += szData;
				}
			}
		}
		else
		{
			RPT_szCRLF = bAccel ? "" : RPT_CRLF;
			for (uint32_t j = 0; j < nSCount; j++)
			{
				SENFRAMEDATA*  pSD = &(sarr[j]);
				float *pAccel = 0;
				char b0, b1;
				b0 = '0' + (pSD->SFinfo.bfBtnState0);
				b1 = '0' + (pSD->SFinfo.bfBtnState1);
				uint8_t dist = pSD->SFinfo.bfDistortion;
				uint16_t aux = pSD->SFinfo.bfAuxInput;

				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}

				//seu sens frame
				sprintf_s(szData, _countof(szData),
					"%2d  %2d  %09d  ",
					nSeuID + nAdd, pSD->SFinfo.bfSnum + nAdd,
					nFrameNum);
				s += szData;

				// bf: stylus
				sprintf_s(szData, _countof(szData),
					"%c%c  ",
					b0, b1);
				s += szData;
				//// bf: dax
				//sprintf_s(szData, _countof(szData),
				//	"%03d  %04x  ",
				//	dist,aux);
				//s += szData;


				//pos
#ifndef _EP
				sprintf_s(szData, _countof(szData),
					//"  %- 9.3f %- 9.3f %- 9.3f ", //orig
					//"  %-09.3f %-09.3f %-09.3f ",
					"  %- 9.3f %- 9.3f %- 9.3f ",
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
#else
				sprintf_s(szData, _countof(szData),
					"  %- 9.5f %- 9.5f %- 9.5f ",
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
#endif
				s += szData;
#ifndef _EP
				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData),
						" %- 8.4f %- 8.4f %- 8.4f %- 8.4f %s",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3],
						RPT_szCRLF);
				}
				else
				{
					sprintf_s(szData, _countof(szData),
						//" %- 8.3f %- 8.3f %- 8.3f %s", //orig
						  " %- 8.3f %- 8.3f %- 8.3f %s",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2],
						RPT_szCRLF);
				}
#else
				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData),
						" %- 8.6f %- 8.6f %- 8.6f %- 8.6f %s",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3],
						RPT_szCRLF);
				}
				else
				{
					sprintf_s(szData, _countof(szData),
						" %- 8.5f %- 8.5f %- 8.5f %s",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2],
						RPT_szCRLF);
				}

#endif
				s += szData;

				if (bAccel)
				{
#ifndef _EP
					if (!pAccel)
					{
						sprintf_s(szData, _countof(szData),
							"%s"
							, RPT_CRLF);
					}
					else
					{
						sprintf_s(szData, _countof(szData),
							//" % 8.3f  % 8.3f  % 8.3f  % 8.3f %s"
							  " %- 8.3f  %- 8.3f  %- 8.3f  %- 8.3f %s"
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]
							, RPT_CRLF);
					}
#else
					if (!pAccel)
					{
						sprintf_s(szData, _countof(szData),
							"%s"
							, RPT_CRLF);
					}
					else
					{
						sprintf_s(szData, _countof(szData),
							" % 8.5f  % 8.5f  % 8.5f  % 8.5f %s"
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]
							, RPT_CRLF);
					}
#endif
					s += szData;
				}

			}
		}

	}
	void CSVHeader(std::string & s, bool bOneline = true, bool bAccel = true)
	{
		MakePrintable();

		uint32_t	nSCount = seupno.sensorCount;
		uint32_t	nPnoMode = seupno.HPinfo.bfPnoMode;
		float *pAccel = 0;

		char szData[500];
		CVPSnsFrameA sd_a;
		if (bOneline)
		{
			sprintf_s(szData, _countof(szData), "SEU, Frame");
			s += szData;

			for (uint32_t j = 0; j < nSCount; j++)
			{
				s += ", ";
				SENFRAMEDATA*  pSD = &(sarr[j]);
				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}
				sprintf_s(szData, _countof(szData), "Sensor, btn0, btn1, distortion, aux(hex), X, Y, Z, ");
				s += szData;


				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData), "Qw, Qx, Qy, Qz");
					s += szData;
				}
				else
				{
					sprintf_s(szData, _countof(szData), "Az, El, Rl, ");
					s += szData;
				}
				if (bAccel && pAccel)
				{
					sprintf_s(szData, _countof(szData), ", a(X), a(Y), a(Z), a(R)");
					s += szData;
				}
				//s += szData;
			}
		}
		else
		{
			sprintf_s(szData, _countof(szData), "SEU, Frame, ");
			s += szData;
			sprintf_s(szData, _countof(szData), "Sensor, btn0, btn1, distortion, aux(hex), X, Y, Z, ");
			s += szData;

			SENFRAMEDATA *  pSD = &(sarr[0]);
			if (nPnoMode)
			{
				sd_a.Fill((SENFRAMEDATA_A*)pSD);
				pSD = sd_a;
				pAccel = sd_a.accel;
			}
			if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
			{
				sprintf_s(szData, _countof(szData), "Qw, Qx, Qy, Qz");
				s += szData;
			}
			else
			{
				sprintf_s(szData, _countof(szData), "Az, El, Rl, ");
				s += szData;
			}
			//s += szData;
			if (bAccel && pAccel)
			{
				sprintf_s(szData, _countof(szData), ",aX, aY, aZ, aRng");
				s += szData;
			}

		}

	}
#ifndef _EP
	void CSVReport(std::string & s, bool bOneline = true, bool bAccel=true)
	{
		MakePrintable();

		uint32_t	nSeuID = seupno.seuid;
		uint32_t	nFrameNum = seupno.frame;
		uint32_t	nSCount = seupno.sensorCount;
		uint32_t	nPnoMode = seupno.HPinfo.bfPnoMode;

		char szData[500];
		CVPSnsFrameA sd_a;
		if (bOneline)
		{
			sprintf_s(szData, _countof(szData), "%3d, %07d", nSeuID, nFrameNum);
			s += szData;

			for (uint32_t j = 0; j < nSCount; j++)
			{
				s += ", ";
				SENFRAMEDATA*  pSD = &(sarr[j]);
				float *pAccel = 0;
				char b0, b1;
				b0 = '0' + (pSD->SFinfo.bfBtnState0);
				b1 = '0' + (pSD->SFinfo.bfBtnState1);
				uint8_t dist = pSD->SFinfo.bfDistortion;
				uint16_t aux = pSD->SFinfo.bfAuxInput;
				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}

				sprintf_s(szData, _countof(szData), "%3d, %c, %c, %d, %x, % 7.3f, % 7.3f, % 7.3f, ",
					pSD->SFinfo.bfSnum, b0, b1, dist, aux,
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
				s += szData;

				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData), "  % 8.4f, % 8.4f, % 8.4f, % 8.4f",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
				}
				else
				{
					sprintf_s(szData, _countof(szData), " % 8.4f, % 8.4f, % 8.4f, ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
				}
				s += szData;
				if (bAccel)
				{
					if (!pAccel)
					{
					}
					else
					{
						sprintf_s(szData, _countof(szData),
							", % 8.3f,% 8.3f,% 8.3f,% 8.3f"
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
					}
					s += szData;
				}
			}
		}
		else // multi-line
		{
			for (uint32_t j = 0; j < nSCount; j++)
			{
				SENFRAMEDATA *  pSD = &(sarr[j]);
				float *pAccel = 0;
				char b0, b1;
				b0 = '0' + (pSD->SFinfo.bfBtnState0);
				b1 = '0' + (pSD->SFinfo.bfBtnState1);
				uint8_t dist = pSD->SFinfo.bfDistortion;
				uint16_t aux = pSD->SFinfo.bfAuxInput;
				if (nPnoMode)
				{
					sd_a.Fill((SENFRAMEDATA_A*)pSD);
					pSD = sd_a;
					pAccel = sd_a.accel;
				}

				sprintf_s(szData, _countof(szData), "%3d, %07d, %3d, %c, %c, %d, %x, % 7.3f, % 7.3f, % 7.3f, ",
					nSeuID, nFrameNum, pSD->SFinfo.bfSnum, b0, b1, dist, aux,
					pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
				s += szData;

				if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
				{
					sprintf_s(szData, _countof(szData), " % 8.4f, % 8.4f, % 8.4f, % 8.4f ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
				}
				else
				{
					sprintf_s(szData, _countof(szData), " % 8.3f, % 8.3f, % 8.3f, ",
						pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
				}
				s += szData;
				if (bAccel)
				{
					if (!pAccel)
					{
						sprintf_s(szData, _countof(szData),
							", No_Accel,");
					}
					else
					{
						sprintf_s(szData, _countof(szData),
							", % 8.3f,% 8.3f,% 8.3f,% 8.3f "
							, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
					}
					s += szData;
				}
				s += "\r\n";
			}
		}

	}
#else
void CSVReport(std::string & s, bool bOneline = true, bool bAccel = false)
{
	MakePrintable();

	uint32_t	nSeuID = seupno.seuid;
	uint32_t	nFrameNum = seupno.frame;
	uint32_t	nSCount = seupno.sensorCount;
	uint32_t	nPnoMode = seupno.HPinfo.bfPnoMode;

	char szData[500];
	CVPSnsFrameA sd_a;
	if (bOneline)
	{
		sprintf_s(szData, _countof(szData), "%3d, %07d, ", nSeuID, nFrameNum);
		s += szData;

		for (uint32_t j = 0; j < nSCount; j++)
		{
			SENFRAMEDATA*  pSD = &(sarr[j]);
			float *pAccel = 0;
			char b0, b1;
			b0 = '0' + (pSD->SFinfo.bfBtnState0);
			b1 = '0' + (pSD->SFinfo.bfBtnState1);
			uint8_t dist = pSD->SFinfo.bfDistortion;
			uint16_t aux = pSD->SFinfo.bfAuxInput;
			if (nPnoMode)
			{
				sd_a.Fill((SENFRAMEDATA_A*)pSD);
				pSD = sd_a;
				pAccel = sd_a.accel;
			}

			sprintf_s(szData, _countof(szData), "%3d, %c, %c, %d, %x, % 9.5f, % 9.5f, % 9.5f, ",
				pSD->SFinfo.bfSnum, b0, b1, dist, aux,
				pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
			s += szData;

			if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
			{

				sprintf_s(szData, _countof(szData), "  % 10.6f, % 10.6f, % 10.6f, % 10.6f, ",
					pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
			}
			else
			{
				sprintf_s(szData, _countof(szData), " % 10.6f, % 10.6f, % 10.6f, , ",
					pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
			}
			s += szData;
			if (bAccel)
			{
				if (!pAccel)
				{
					sprintf_s(szData, _countof(szData),
						" No_Accel,");
				}
				else
				{
					sprintf_s(szData, _countof(szData),
						" % 10.5f,% 10.5f,% 10.5f,% 10.5f \r\n"
						, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
				}
				s += szData;
			}
		}
	}
	else
	{
		for (uint32_t j = 0; j < nSCount; j++)
		{
			SENFRAMEDATA *  pSD = &(sarr[j]);
			float *pAccel = 0;
			char b0, b1;
			b0 = '0' + (pSD->SFinfo.bfBtnState0);
			b1 = '0' + (pSD->SFinfo.bfBtnState1);
			uint8_t dist = pSD->SFinfo.bfDistortion;
			uint16_t aux = pSD->SFinfo.bfAuxInput;
			if (nPnoMode)
			{
				sd_a.Fill((SENFRAMEDATA_A*)pSD);
				pSD = sd_a;
				pAccel = sd_a.accel;
			}

			sprintf_s(szData, _countof(szData), "%3d, %07d, %3d, %c, %c, %d, %x, % 9.5f, % 9.5f, % 9.5f, ",
				nSeuID, nFrameNum, pSD->SFinfo.bfSnum, b0, b1, dist, aux,
				pSD->pno.pos[0], pSD->pno.pos[1], pSD->pno.pos[2]);
			s += szData;

			if (pSD->SFinfo.bfOriUnits == ORI_QUATERNION)
			{
				sprintf_s(szData, _countof(szData), " % 10.6f, % 10.6f, % 10.6f, % 10.6f\r\n",
					pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2], pSD->pno.ori[3]);
			}
			else
			{
				sprintf_s(szData, _countof(szData), " % 10.5f, % 10.5f, % 10.5f, \r\n",
					pSD->pno.ori[0], pSD->pno.ori[1], pSD->pno.ori[2]);
			}
			s += szData;
			if (bAccel)
			{
				if (!pAccel)
				{
					sprintf_s(szData, _countof(szData),
						" No_Accel,");
				}
				else
				{
					sprintf_s(szData, _countof(szData),
						" % 10.5f,% 10.5f,% 10.5f,% 10.5f \r\n"
						, pAccel[0], pAccel[1], pAccel[2], pAccel[3]);
				}
				s += szData;
			}
		}
	}

}

#endif
};

//typedef struct WHOAMI_STRUCT
//{
//	char device_name[NAME_SIZE];
//	char hw_ser_no[SERNUM_SIZE];
//	char ioproc_pn[PN_SIZE];
//	char dsp_bt_fw_pn[PN_SIZE];
//	char dsp_app_fw_pn[PN_SIZE];
//}WHOAMI_STRUCT;
class VPCMD_API CWhoAmI : public WHOAMI_STRUCT
{
public:
	CWhoAmI()
	{
		Init();
	}

	CWhoAmI( const CWhoAmI & rv)
	{
		memcpy(device_name, rv.device_name, sizeof(WHOAMI_STRUCT));
	}

	CWhoAmI(const WHOAMI_STRUCT * prv)
	{
		memcpy(device_name, prv, sizeof(WHOAMI_STRUCT));
	}

	operator WHOAMI_STRUCT * () { return (WHOAMI_STRUCT *)this; }

	void Init()
	{
		memset(device_name, 0, sizeof(WHOAMI_STRUCT));
	}

	size_t Size() { return sizeof(WHOAMI_STRUCT); }

	void Fill(const char* name = 0, const char * hwser = 0, const char* iop = 0, const char * dspbt = 0, const char * dspapp = 0)
	{
		if (name)
			memcpy(device_name, name, sizeof(device_name));

		if (hwser)
			memcpy(hw_ser_no, hwser, sizeof(hw_ser_no));

		if (iop)
			memcpy(ioproc_pn, iop, sizeof(ioproc_pn));

		if (dspbt)
			memcpy(dsp_bt_fw_pn, dspbt, sizeof(dsp_bt_fw_pn));

		if (dspapp)
			memcpy(dsp_app_fw_pn, dspapp, sizeof(dsp_app_fw_pn));
	}

	void MakePrintable()
	{
		device_name[NAME_SIZE - 1] = 0;
		hw_ser_no[SERNUM_SIZE - 1] = 0;
		ioproc_pn[PN_SIZE - 1] = 0;
		dsp_bt_fw_pn[PN_SIZE - 1] = 0;
		dsp_app_fw_pn[PN_SIZE - 1] = 0;
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[300];
		sprintf_s(sz, 300,
			"%s" "%s"
			"%s" "device_name: %s" "%s"
			"%s" "hw_ser_no:   %s" "%s"
			"%s" "ioproc:      %s" "%s"
			"%s" "dsp_bt_fw:   %s" "%s"
			"%s" "dsp_app_fw:  %s\r\n",
			bTitle ? "WHOAMI_STRUCT: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, device_name, RPT_szCRLF,
			RPT_szINDENT, hw_ser_no, RPT_szCRLF,
			RPT_szINDENT, ioproc_pn, RPT_szCRLF,
			RPT_szINDENT, dsp_bt_fw_pn, RPT_szCRLF,
			RPT_szINDENT, dsp_app_fw_pn);
		s += sz;
	}
};

//typedef struct _STATION_MAP_CONFIG
//{
//	uint32_t station_map;	// read-only.  Ignored by tracker in set action
//	uint32_t enabled_map;
//}STATION_MAP_CONFIG;


//typedef struct _STATION_MAP
//{
//	union
//	{
//		uint32_t stamap;
//		struct _bf {
//			uint32_t sensor_map : 16;
//			uint32_t reserved1 : 8;
//			uint32_t source_map : 4;
//			uint32_t reserved2 : 4;
//		} bf;
//	};
//}STATION_MAP;
class VPCMD_API CStationMap : public STATION_MAP
{
public:
	uint32_t sns_detected_count;  //<! Detected sensor count
	uint32_t src_detected_count;  //<! Detected source count
	uint32_t en_count;        //<! Enabled sensor count (enabled & detected)
	uint32_t en_map;          //<! Enabled sensor map.  For host use and maintenance only!

	CStationMap()
	{
		Init();
	}

	CStationMap(const CStationMap & rv)
	{
		//memcpy(&sensor_map, &rv.sensor_map, sizeof(STATION_MAP));
		stamap = rv.stamap;
		en_count = rv.en_count;
		en_map = rv.en_map;
		CountDetected();

	}

	CStationMap(const STATION_MAP * prv)
	{
		//memcpy(&station_map, prv, sizeof(STATION_MAP_CONFIG));
		stamap = prv->stamap;
		CountDetected();
		en_count = sns_detected_count;
		en_map = bf.sensor_map;
	}

	CStationMap(uint32_t map)
	{
		stamap = map;
		CountDetected();
		en_count = sns_detected_count;
		en_map = bf.sensor_map;
	}

	operator STATION_MAP * () { return (STATION_MAP *)this; }
	operator void * () { return (void *)((STATION_MAP *)this); }
	bool operator == (const STATION_MAP *prv) { return (prv->stamap == stamap) ; }

	void Init()
	{
		memset(&stamap, 0, sizeof(STATION_MAP));
		CountDetected();
		InitEnabled();
	}

	size_t Size() { return sizeof(STATION_MAP); }

	void CountDetected()
	{
		sns_detected_count = 0;
		for (int i = 0; i < SENSORS_PER_SEU; i++)
		{
			if ((1 << i) & bf.sensor_map)
				sns_detected_count++;
		}
		src_detected_count = 0;
		for (int i = 0; i < SOURCES_PER_SEU; i++)
		{
			if ((1 << i) & bf.source_map)
				src_detected_count++;
		}

	}
	void CountEnabled()
	{
		CountDetected();
		en_count = 0;
		for (int i = 0; i < SENSORS_PER_SEU; i++)
		{
			if ((1 << i) & (en_map))
				en_count++;
		}
	}
	void InitEnabled()
	{
		en_map = bf.sensor_map;
		CountEnabled();
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(STATION_MAP));
		CountDetected();
		InitEnabled();
	}
	uint32_t SensorMap() { return bf.sensor_map; }
	uint32_t SourceMap() { return bf.source_map; }

	uint32_t & EnabledMap()  { return en_map; }

	//bool IsEnabled(int32_t sns)  { return ((1 << sns) & enabled_map) != 0; }
	bool IsDetected(int32_t sns) { return ((1 << sns) & bf.sensor_map) != 0; }
	bool IsEnabled(int32_t sns)   { return ((1 << sns) & (en_map & bf.sensor_map)) != 0; }
	bool IsSrcDetected(int32_t src) { return ((1 << src) & bf.source_map) != 0; }
	uint32_t SnsDetectedCount() { CountDetected(); return sns_detected_count; }
	void SetEnabled(uint32_t s) 
	{	
		en_map &= (1 << s); 
		CountEnabled(); 
	}
	//void SetProcEnMap(int p, uint8_t map)
	//{
	//	uint32_t mask = ~(0xf << p);
	//	enabled_map &= mask; // clear the map for this p
	//	enabled_map |= (map << p);
	//	CountActive();
	//}

	//void Fill(uint32_t stamap = 0, uint32_t enmap = 0) // don't need to Fill.  It is read-only
	//{
	//	station_map = stamap;
	//	enabled_map = enmap;
	//	CountActive();
	//}

	void MakePrintable()
	{

	}

	//void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	//{
	//	RPT_szCRLF = bFlat ? "" : RPT_CRLF;
	//	RPT_szINDENT = bTitle ? "   " : "";
	//
	//	MakePrintable();
	//	char sz[200];
	//	sprintf_s(sz, 200,
	//		"%s" "%s"
	//		"%s" "sens procs : % 6d" "%s"
	//		"%s" "station_map: 0x%03x" "%s"
	//		"%s" "enabled_map: 0x%04x\r\n",
	//		bTitle ? "STATION_MAP_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
	//		RPT_szINDENT, snsprocs, RPT_szCRLF,
	//		RPT_szINDENT, sensormap, RPT_szCRLF,
	//		RPT_szINDENT, enabled_map);
	//	s += sz;
	//}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "sensor_map: 0x%02x" "%s"
			"%s" "source_map: 0x%01x\r\n",
			bTitle ? "STATION_MAP: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, bf.sensor_map, RPT_szCRLF,
			RPT_szINDENT, bf.source_map);
		s += sz;
	}
	void ReportCounts(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		CountDetected();
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "sensor count: % 2d" "%s"
			"%s" "source count: % 1d\r\n",
			bTitle ? "STATION_MAP: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, sns_detected_count, RPT_szCRLF,
			RPT_szINDENT, src_detected_count);
		s += sz;
	}

	void ReportEn(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "sensor_map:  0x%02x" "%s"
			"%s" "source_map:  0x%01x" "%s"
			"%s" "enabled_map: 0x%02x" "\r\n",
			bTitle ? "STATION_MAP: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, bf.sensor_map, RPT_szCRLF,
			RPT_szINDENT, bf.source_map, RPT_szCRLF,
			RPT_szINDENT, en_map
		);
		s += sz;
	}

	static STATION_MAP Default;
};

//typedef struct _UNITS_CONFIG
//{
//	uint32_t pos_units;		// valid values in enum eViperPosUnits
//	uint32_t ori_units;		// valid values in enum eViperOriUnits
//}UNITS_CONFIG;
class VPCMD_API CUnitsCfg : public UNITS_CONFIG
{
public:
	CUnitsCfg()
	{
		Init();
	}

	CUnitsCfg(const CUnitsCfg & rv)
	{
		memcpy(&pos_units, &rv.pos_units, sizeof(UNITS_CONFIG));
	}

	CUnitsCfg(const UNITS_CONFIG * prv)
	{
		memcpy(&pos_units, prv, sizeof(UNITS_CONFIG));
	}
	CUnitsCfg(eViperPosUnits posu, eViperOriUnits oriu)
	{
		pos_units = (uint32_t)posu;
		ori_units = (uint32_t)oriu;
	}

	operator UNITS_CONFIG * () { return (UNITS_CONFIG *)this; }
	operator void * () { return (void *)((UNITS_CONFIG *)this); }
	bool operator == (const UNITS_CONFIG *prv) const { return (prv->pos_units == pos_units) && (prv->ori_units == ori_units); }

	void Init()
	{
		InitDefault();
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(UNITS_CONFIG));
	}

	void Fill(uint32_t pos = POS_INCH, uint32_t ori = ORI_QUATERNION)
	{
		pos_units = pos;
		ori_units = ori;
	}

	bool IsEuler() { return ori_units != ORI_QUATERNION; }
	eViperPosUnits ePos() { return (eViperPosUnits)pos_units; }
	eViperOriUnits eOri() { return (eViperOriUnits)ori_units; }
	const char *posunitsstr(bool bpub = false) { return CVPcmd::posunitsstr(pos_units, bpub); }
	const char *oriunitsstr(bool bpub = false) { return CVPcmd::oriunitsstr(ori_units, bpub); }

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "pos_units: %s" "%s"
			"%s" "ori_units: %s\r\n",
			bTitle ? "UNITS_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, CVPcmd::posunitsstr(pos_units), RPT_szCRLF,
			RPT_szINDENT, CVPcmd::oriunitsstr(ori_units));
		s += sz;
	}
	void ShortReport(std::string & s)
	{
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s, %s",
			CVPcmd::posunitsstr(pos_units), CVPcmd::oriunitsstr(ori_units));
		s += sz;
	}
	static UNITS_CONFIG Default;
};

//typedef struct _SRCROT_CONFIG
//{
//	uint32_t srcindex;
//	float rot[4];
//}SRCROT_CONFIG;
class VPCMD_API CSrcRotCfg : public SRCROT_CONFIG
{
public:
	CSrcRotCfg()
	{
		Init();
	}

	CSrcRotCfg(const CSrcRotCfg & rv)
	{
		memcpy(&srcfreq, &(rv.srcfreq), sizeof(SRCROT_CONFIG));
		m_eOriUnits = rv.m_eOriUnits;
	}

	CSrcRotCfg(const SRCROT_CONFIG * prv)
	{
		Init();
		memcpy(&srcfreq, prv, sizeof(SRCROT_CONFIG));
	}

	CSrcRotCfg(const SRC_CONFIG * prv)
	{
		Init();
		srcfreq = prv->srcfreq;
		memcpy(&rot[0], prv->srcpno.ori, sizeof(rot));
	}

	CSrcRotCfg(float p[4])
	{
		Init();
		memcpy(rot, p, 4 * sizeof(float));
	}

	operator SRCROT_CONFIG * () { return (SRCROT_CONFIG *)this; }
	operator void * () { return (void *)((SRCROT_CONFIG *)this); }
	bool operator ==  (const float *rv) const
	{
		return (FP_EQ(rot[0], rv[0]) && FP_EQ(rot[1], rv[1]) && FP_EQ(rot[2], rv[2]) && FP_EQ(rot[3], rv[3]));
	}

	CSrcRotCfg & operator= (const CSrcRotCfg & rv)
	{
		memcpy(&srcfreq, &(rv.srcfreq), sizeof(SRCROT_CONFIG));
		m_eOriUnits = rv.m_eOriUnits;
		return *this;
	}

	CSrcRotCfg & operator= (const SRCROT_CONFIG * prv)
	{
		memcpy(&srcfreq, prv, sizeof(SRCROT_CONFIG));
		return *this;
	}

	CSrcRotCfg & operator= (const float p[4])
	{
		memcpy(&rot, p, 4 * sizeof(float));
		return *this;
	}

	void Init()
	{
		memset(&srcfreq, 0, sizeof(SRCROT_CONFIG));
		m_eOriUnits = CUnitsCfg().eOri();
	}
	void InitDefault()
	{
		memcpy(this, &Default[m_eOriUnits], sizeof(SRCROT_CONFIG));
	}

	void Fill(float x = 0, float y = 0, float z = 0, float q4 = 0, int srcf = 0)
	{
		Init();
		rot[0] = x;
		rot[1] = y;
		rot[2] = z;
		rot[3] = q4;
		srcfreq = srcf;
	}

	uint32_t SrcFreq() { return srcfreq; } //read-only
	float & Az() { return rot[0]; }
	float & El() { return rot[1]; }
	float & Ro() { return rot[2]; }
	float & Qw() { return rot[0]; }
	float & Qx() { return rot[1]; }
	float & Qy() { return rot[2]; }
	float & Qz() { return rot[3]; }

	void Fill(float *p)
	{
		memcpy(rot, p, 4 * sizeof(float));
	}

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[200];
		//uint32_t leU = (eU == ORI_MAX) ? m_eOriUnits : eU;
		//const char * szOriU = CVPcmd::oriunitsstr(leU);
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "srcfreq, (w,x,y,z): %d, (%f, %f, %f, %f)\r\n",
			bTitle ? "SRCROT_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, srcfreq, rot[0], rot[1], rot[2], rot[3]);
		s += sz;
	}
	// units attribute is only used for a couple of purposes.  
	// It is for carrying the information around; not really for acting on it.
	// It is not filled by fill operation.
	// It is not copied by operator = or by copy constructor unless from another CVPBoresight object
	// It is initialized during construction.
	// It is used for the Report operation, if a valid units value is not supplied as an argument.
	// It is used for Enabled state operation
	eViperOriUnits m_eOriUnits;
	eViperOriUnits & Units() { return m_eOriUnits; }

	static SRCROT_CONFIG Default[ORI_MAX];
};

//typedef struct _PNO {
//	float pos[3];
//	float ori[4];
//}PNODATA;
//typedef struct _SRC_CONFIG
//{
//	uint32_t srcfreq;
//	PNODATA srcpno;
//	uint32_t startuphem;
//}SRC_CONFIG;
class VPCMD_API CSrcCfg : public SRC_CONFIG
{
public:

	CSrcCfg()
	{
		Init();
	}

	CSrcCfg(const CSrcCfg & rv)
	{
		memcpy(&srcfreq, &(rv.srcfreq), sizeof(SRC_CONFIG));
		m_units = rv.m_units;
	}

	CSrcCfg(const SRC_CONFIG * prv)
	{
		Init();
		memcpy(&srcfreq, prv, sizeof(SRC_CONFIG));
	}
	CSrcCfg(const SRCROT_CONFIG * prv)
	{
		Init();
		srcfreq = prv->srcfreq;

		memcpy(srcpno.ori, prv->rot, 4 * sizeof(float));
	}

	CSrcCfg(float p[7])
	{
		Init();
		memcpy(&srcpno, p, 7 * sizeof(float));
	}

	operator SRC_CONFIG * () { return (SRC_CONFIG *)this; }
	operator void * () { return (void *)((SRC_CONFIG *)this); }
	//bool operator == (const UNITS_CONFIG *prv) const { return (prv->pos_units == pos_units) && (prv->ori_units == ori_units); }
	bool operator ==  (const SRC_CONFIG *prv) const
	{
		return (prv->srcfreq == srcfreq);// && FP_EQ(rot[0], rv[0]) && FP_EQ(rot[1], rv[1]) && FP_EQ(rot[2], rv[2]) && FP_EQ(rot[3], rv[3]));
	}

	CSrcCfg & operator= (const CSrcCfg & rv)
	{
		memcpy(&srcfreq, &(rv.srcfreq), sizeof(SRC_CONFIG));
		m_units = rv.m_units;
		return *this;
	}

	CSrcCfg & operator= (const SRC_CONFIG * prv)
	{
		memcpy(&srcfreq, prv, sizeof(SRC_CONFIG));
		return *this;
	}

	CSrcCfg & operator= (const float p[4])
	{
		memcpy(&srcpno.ori, p, 4 * sizeof(float));
		return *this;
	}

	void Init()
	{
		memset(&srcfreq, 0, sizeof(SRC_CONFIG));
		m_units.Init();// = CUnitsCfg(POS_MAX, ORI_MAX);
	}
	void InitDefault()
	{
		memcpy(this, &Default[m_units.eOri()], sizeof(SRC_CONFIG));
	}

	void Fill(int srcf = 0, PNODATA* p = 0 )
	{
		Init();
		srcfreq = srcf;
		if (p)
			memcpy(&srcpno, p, sizeof(PNODATA));
	}

	uint32_t SrcFreq() { return srcfreq; } //read-only
	uint32_t & StartupHem() { return startuphem; }
	float & Az() { return srcpno.ori[0]; }
	float & El() { return srcpno.ori[1]; }
	float & Ro() { return srcpno.ori[2]; }
	float & Qw() { return srcpno.ori[0]; }
	float & Qx() { return srcpno.ori[1]; }
	float & Qy() { return srcpno.ori[2]; }
	float & Qz() { return srcpno.ori[3]; }
	float & X() { return srcpno.pos[0]; }
	float & Y() { return srcpno.pos[1]; }
	float & Z() { return srcpno.pos[2]; }

	bool IsNull()
	{
		CSrcCfg nullcfg(&Default[m_units.eOri()]);
		return ComparePos(nullcfg) && CompareOri(nullcfg);
	}
	bool ComparePos(const CSrcCfg & rv) const
	{
		return (FP_EQ(srcpno.pos[0], rv.srcpno.pos[0]) && FP_EQ(srcpno.pos[1], rv.srcpno.pos[1]) && FP_EQ(srcpno.pos[2], rv.srcpno.pos[2])) ;
	}
	bool CompareOri(const CSrcCfg & rv) const
	{
		return (FP_EQ(srcpno.ori[0], rv.srcpno.ori[0]) && FP_EQ(srcpno.ori[1], rv.srcpno.ori[1]) && FP_EQ(srcpno.ori[2], rv.srcpno.ori[2]) && FP_EQ(srcpno.ori[3], rv.srcpno.ori[3]));
	}
	void Fill(float *p)
	{
		memcpy(&srcpno, p, 7 * sizeof(float));
	}

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";

		MakePrintable();
		char sz[200];

		if (m_units.IsEuler())
			sprintf_s(sz, 200,
				"%s" "%s"
				"%s" "srcfreq, pos(x,y,z) %s ori(x,y,z) %s: %d, (%f, %f, %f), (%f, %f, %f), %s\r\n",
				bTitle ? "SRC_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, m_units.posunitsstr(), m_units.oriunitsstr(),
				srcfreq, X(), Y(), Z(), Az(), El(), Ro(), CVPcmd::vphemispherestr(startuphem) );
		else
			sprintf_s(sz, 200,
				"%s" "%s"
				"%s" "srcfreq, pos(x,y,z) %s ori(w,x,y,z): %d, (%f, %f, %f), (%f, %f, %f, %f), %s\r\n",
				bTitle ? "SRC_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, m_units.posunitsstr(), 
				srcfreq, X(), Y(), Z(), Qw(), Qx(), Qy(), Qz(), CVPcmd::vphemispherestr(startuphem));
		s += sz;
	}
	// units attribute is only used for a couple of purposes.  
	// It is for carrying the information around; not really for acting on it.
	// It is not filled by fill operation.
	// It is not copied by operator = or by copy constructor unless from another CSrcCfg object
	// It is initialized during construction.
	// It is used for the Report operation, if a valid units value is not supplied as an argument.
	CUnitsCfg m_units;
	CUnitsCfg & Units() { return m_units; }

	static SRC_CONFIG Default[ORI_MAX];
};


//typedef struct _FRAMERATE_CONFIG
//{
//	uint32_t divisor;
//}FRAMERATE_CONFIG;
#define VIPER_HW_MAXFRATE 960
class VPCMD_API CFrameRateCfg //: public FRAMERATE_CONFIG
{
public:
	//eViperFrameRate eFR;
	FRAMERATE_CONFIG frame_rate;

	CFrameRateCfg()
	{
		Init();
	}

	CFrameRateCfg(const CFrameRateCfg & rv)
	{
		frame_rate = rv.frame_rate;
	}

	CFrameRateCfg(const FRAMERATE_CONFIG * prv)
	{
		memcpy(&frame_rate, prv, sizeof(FRAMERATE_CONFIG));
	}

	operator FRAMERATE_CONFIG * () { return &frame_rate; }
	operator void * () { return (void *)(&frame_rate); }
	bool operator == (FRAMERATE_CONFIG *prv) const { return *prv == frame_rate; }

	//uint32_t EffectiveRate() { return VIPER_MAX_FRAMES_PER_SEC / (divisor ? divisor : 1); }
	uint32_t EffectiveRate() { return (uint32_t)(VIPER_HW_MAXFRATE / pow(2, FR_MAX - frame_rate - 1)); }// ((divisor ? divisor : 1);
	uint32_t MaxRate() { return VIPER_MAX_FRAMES_PER_SEC; }

	void Init()
	{
		frame_rate = FR_240;
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(FRAMERATE_CONFIG));
	}

	void Fill(uint32_t fr)
	{
		frame_rate = CLAMP((int)fr, FR_30, FR_960);
	}

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle=false, bool bFlat=false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "effective frame rate: %d Hz" "%s"
			"%s" "frame_rate: %s" "%s"
			"%s" "max frame rate: %d\r\n",
			bTitle ? "FRAMERATE_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, EffectiveRate(), RPT_szCRLF,
			RPT_szINDENT, CVPcmd::frameratestr(frame_rate), RPT_szCRLF,
			RPT_szINDENT, MaxRate());
		s += sz;
	}

	static FRAMERATE_CONFIG Default;
};

//typedef struct _BIT_STRUCT
//{
//	uint32_t bfSensorErr : 16;   //a 1 in any bit shows E_BITERR_SENSOR
//	uint32_t res1 : 16;
//	uint32_t bfSensorPortErr : 16; //a 1 in any bit shows E_BITERR_SENSOR_PORT
//	uint32_t res2 : 16;
//	uint32_t bfSourceErr : 4;  //a 1 in any bit shows E_BITERR_SOURCE
//	uint32_t res3 : 4;
//	uint32_t res4 : 24;
//	uint32_t bfSEUErr : 4;  //a 1 in any bit shows sensor board E_BITERR_SEU
//	uint32_t res5 : 4;
//	uint32_t res6 : 24;
//	uint32_t bfDupFreqErr : 4;  //a 1 in any bit shows sources with E_BITERR_DUPFREQ
//	uint32_t res7 : 4;
//	uint32_t res8 : 24;
//	uint32_t bfReserved1 : 32;
//	uint32_t bfReserved2 : 32;
//}BIT_STRUCT;
typedef struct {
	union {
		uint32_t errs[E_BITERR_MAX - 1];
		BIT_STRUCT bf;
	};
}BIT_STRUCTu;
class VPCMD_API CBitResult : public BIT_STRUCTu
{
public:
	uint32_t errmap;
	uint32_t errcount;

	CBitResult()
	{
		Init();
	}
	size_t Size() { return sizeof(BIT_STRUCTu); }
	CBitResult(const CBitResult & rv)
	{
		memcpy(errs, rv.errs, Size());
		SetMap();
	}

	CBitResult(const BIT_STRUCT * prv)
	{
		memcpy(&bf, prv, Size());
		SetMap();
	}
	
	operator BIT_STRUCT * () { return (BIT_STRUCT *)this; }
	operator void * () { return (void *)((BIT_STRUCT *)this); }

	void SetMap()
	{
		errcount = 0;
		errmap = 0;
		for (int i = 0; i < E_BITERR_MAX - 1; i++)
		{
			if (errs[i])
			{
				errmap |= (1 << (i + 1));
				errcount++;
			}
		}
	}
	void Init()
	{
		memset(errs, 0, Size());
		SetMap();
	}
	void InitDefault()
	{
		Init();
	}

	void MakePrintable()
	{	}
	
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		if (errcount == 0)
		{
			sprintf_s(sz, 200,
				"%s" "%s"
				"%s" "%s\r\n",
				bTitle ? "BIT_STRUCT: " : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, CVPcmd::bitcodestr(0));
			s += sz;
		}
		else
		{
			sprintf_s(sz, 200,
				"%s" "%s",
				bTitle ? "BIT_STRUCT: " : "", bTitle ? RPT_szCRLF : ""
			);
			s += sz;
			for (int i = 0; i < E_BITERR_MAX-1; i++)
			{
				if (errs[i])
				{
					sprintf_s(sz, 200,
						"%s" "%-20s" " :  0x%x" "%s",
						RPT_szINDENT,
						CVPcmd::bitcodestr(i + 1),
						errs[i],
						RPT_szCRLF
					);
					s += sz;
				}
			}
			//sprintf_s(sz, 200,
			//	"%s" "%s"
			//	"%s" "%s" "%s" 
			//	"%s" "%s" "%s" 
			//	"%s" "%s" "%s" 
			//	"%s" "%s" "%s" 
			//	"%s" "%s" " \r\n",
			//	bTitle ? "BIT_STRUCT: " : "", bTitle ? RPT_szCRLF : "",
			//	RPT_szINDENT, errs[E_BITERR_SENSOR - 1] ? CVPcmd::bitcodestr(E_BITERR_SENSOR) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_SENSOR_PORT - 1] ? CVPcmd::bitcodestr(E_BITERR_SENSOR_PORT) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_SOURCE - 1] ? CVPcmd::bitcodestr(E_BITERR_SOURCE) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_SEU - 1] ? CVPcmd::bitcodestr(E_BITERR_SEU) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_DUPFREQ - 1] ? CVPcmd::bitcodestr(E_BITERR_DUPFREQ) : ""
			//	);
			//s += sz;
		}

	}
	void ReportDetail(std::string & s, bool bZerobased = false, bool bTitle = false, bool bFlat = false)
	{
		int32_t     nAdd = (bZerobased) ? 0 : 1;
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = /*bTitle ? "   " :*/ "";
		MakePrintable();
		char sz[200];
		if (errcount == 0)
		{
			sprintf_s(sz, 200,
				"%s" "%s" "%s"
				"%s" "%s\r\n",
				bTitle ? "Built-In Test Results: " : "", bTitle ? RPT_szCRLF : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, CVPcmd::bitcodevalstr(0));
			s += sz;
		}
		else
		{
			sprintf_s(sz, 200,
				"%s" "%s" "%s"
				"%s" "%s",
				bTitle ? "Built-In Test Results: Errors Detected" : "", bTitle ? RPT_szCRLF : "", bTitle ? RPT_szCRLF : "",
				bTitle ? "" : "Errors Detected:" , bTitle? "" : RPT_szCRLF
				);
			s += sz;
			for (int i = 0; i < E_BITERR_MAX - 1; i++)
			{
				if (errs[i])
				{
					char szDet[200];
					std::string sDet;
					int count = 0;
					int beginrun = 0;
					int runcount = 0;
					int endrun = 0;
					// can do +1 here bc reserved bitfields will always be zero so run will end before for loop ends.
					for (int j = 0; j < SENSORS_PER_SEU+1; j++)
					{
						if (errs[i] & (1 << j))
						{
							if( runcount == 0)
								beginrun = j;
							runcount++;
							endrun = j;
							//sprintf_s(szDet, 200, "%s%d", count ? "," : "", j + nAdd);
							//count++;
							//sDet += szDet;
						}
						else
						{
							int chars = 0;
							switch (runcount)
							{
							case 0:
								break;
							case 1:
								chars = sprintf_s(szDet, 200, "%s%d",
									count ? "," : "",
									beginrun + nAdd);
								break;
							case 2:
								chars = sprintf_s(szDet, 200, "%s%d,%d",
									count ? "," : "",
									beginrun + nAdd, endrun+nAdd);
								break;
							default:
								chars = sprintf_s(szDet, 200, "%s%d-%d",
									count ? "," : "", 
									beginrun + nAdd, endrun + nAdd );
								count++;
							}
							if (chars)
							{
								sDet += szDet;
								count++;
							}
							runcount = 0;
							beginrun = 0;
							endrun = 0;
						}
					}
					if (runcount)
					{
						//should never get here.
						sDet += "...";
					}
					sprintf_s(sz, 200,
						//"%s" "%-28s" " :  %s" "%s",
						"%s" "%-15s" " 0x%04x : %s" "%s",
						RPT_szINDENT,
						CVPcmd::bitcodevalstr(i + 1),
						errs[i],
						sDet.c_str(),
						RPT_szCRLF
					);
					s += sz;
				}
			}
			//sprintf_s(sz, 200,
			//	"%s" "%s"
			//	"%s" "%s" "%s" 
			//	"%s" "%s" "%s" 
			//	"%s" "%s" "%s" 
			//	"%s" "%s" "%s" 
			//	"%s" "%s" " \r\n",
			//	bTitle ? "BIT_STRUCT: " : "", bTitle ? RPT_szCRLF : "",
			//	RPT_szINDENT, errs[E_BITERR_SENSOR - 1] ? CVPcmd::bitcodestr(E_BITERR_SENSOR) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_SENSOR_PORT - 1] ? CVPcmd::bitcodestr(E_BITERR_SENSOR_PORT) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_SOURCE - 1] ? CVPcmd::bitcodestr(E_BITERR_SOURCE) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_SEU - 1] ? CVPcmd::bitcodestr(E_BITERR_SEU) : "", RPT_szCRLF,
			//	RPT_szINDENT, errs[E_BITERR_DUPFREQ - 1] ? CVPcmd::bitcodestr(E_BITERR_DUPFREQ) : ""
			//	);
			//s += sz;
		}

	}
	void ReportDetail0(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "errmap: 0x08%x\r\n",
			bTitle ? "BIT_STRUCT: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, errmap);
		s += sz;
	}

};

//typedef struct _FILTER_CONFIG
//{
//	uint32_t level;
//	float params[4];
//}FILTER_CONFIG;
class VPCMD_API CFilterCfg : public FILTER_CONFIG
{
public:
	enum P{ F_SENS, F_LOW, F_HIGH, F_MAXT };
	CFilterCfg()
	{
		Init();
	}

	CFilterCfg(eViperFilterLevel e)
	{
		Init();
		*this = presets[e];
	}

	CFilterCfg(const CFilterCfg & rv)
	{
		memcpy(&level, &rv.level, sizeof(FILTER_CONFIG));
	}

	CFilterCfg(const FILTER_CONFIG * prv)
	{
		memcpy(&level, prv, sizeof(FILTER_CONFIG));
	}

	operator FILTER_CONFIG * () { return (FILTER_CONFIG *)this; }

	operator float * () { return params; }
	operator void * ()  { return (void *)((FILTER_CONFIG *)this); }

	bool operator==          (const CFilterCfg & rv) const
	{
		if (level == rv.level)
		{
			if (Custom())
				return CompareParams(rv);
			else
				return true;
		}
		else
			return false;
	}

	bool CompareParams(const CFilterCfg & rv) const
	{
		return (FP_EQ(params[0], rv.params[0]) && FP_EQ(params[1], rv.params[1]) && FP_EQ(params[2], rv.params[2]) && FP_EQ(params[3], rv.params[3]));
	}

	bool operator==          (const float *rv) const
	{
		return (FP_EQ(params[0],rv[0]) && FP_EQ(params[1],rv[1]) && FP_EQ(params[2],rv[2]) && FP_EQ(params[3],rv[3]));
	}

	CFilterCfg & operator= (const CFilterCfg & rv)
	{
		memcpy(&level, &rv.level, sizeof(FILTER_CONFIG));
		return *this;
	}

	CFilterCfg & operator= (const FILTER_CONFIG * prv)
	{
		memcpy(&level, prv, sizeof(FILTER_CONFIG));
		return *this;
	}

	void Init()
	{
		memset(&level, 0, sizeof(FILTER_CONFIG));
		level = FILTER_LVL_NONE;
		params[1] = 1.0;
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(FILTER_CONFIG));
	}

	void Fill(eViperFilterLevel el, float sens = 0, float flow = 0, float fhigh = 0, float maxt = 0)
	{
		Init();
		level = CLAMP((int)el, (int)FILTER_LVL_NONE, (int)FILTER_LVL_RESERVED);
		switch (level)
		{
			case FILTER_LVL_CUSTOM:
				params[0] = sens;
				params[1] = flow;
				params[2] = fhigh;
				params[3] = maxt;
				break;
			//case FILTER_LVL_NONE:
			//	params[1] = 1.0;
			//	break;
			default:
				// fill with preset values to match preset.
				memcpy(params, presets[level]->params, sizeof(params));
				break;
		}
	}

	void Fill(eViperFilterLevel el, float *p)
	{
		if ((el == FILTER_LVL_CUSTOM) && (p))
			Fill(el, p[0], p[1], p[2], p[3]);
		else
			Fill(el);
	}

	bool None() const { return (level == FILTER_LVL_NONE); }
	bool Custom() const { return (level == FILTER_LVL_CUSTOM); }
	bool Enhanced() const { return level > FILTER_LVL_CUSTOM; }

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "level: %s" "%s"
			"%s" "F:     %f" "%s"
			"%s" "Flow:  %f" "%s"
			"%s" "Fhigh: %f" "%s"
			"%s" "MaxT:  %f\r\n",
			bTitle ? "FILTER_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, CVPcmd::filterlevstr(level), RPT_szCRLF,
			RPT_szINDENT, params[F_SENS], RPT_szCRLF,
			RPT_szINDENT, params[F_LOW], RPT_szCRLF,
			RPT_szINDENT, params[F_HIGH], RPT_szCRLF,
			RPT_szINDENT, params[F_MAXT]);
		s += sz;
	}

	void SetPreset( )
	{
		level = FILTER_LVL_CUSTOM;
		for (int i = 0; i < (int)FILTER_LVL_MAX; i++)
		{
			if (CompareParams(presets[i]))
			{
				level = i; break;
			}
		}
	}

	static FILTER_CONFIG * presets[FILTER_LVL_MAX];

	static FILTER_CONFIG NoneDef;
	static FILTER_CONFIG Light;
	static FILTER_CONFIG Medium;
	static FILTER_CONFIG Heavy;
	static FILTER_CONFIG Default;
	static FILTER_CONFIG ELight;
	static FILTER_CONFIG EMedium;
	static FILTER_CONFIG EHeavy;
	static FILTER_CONFIG EDefault;
};

//typedef struct _TIP_OFFSET_CONFIG
//{
//	float params[3];
//}TIP_OFFSET_CONFIG;
class VPCMD_API CTipoffCfg : public TIP_OFFSET_CONFIG
{
public:
	CTipoffCfg()
	{
		Init();
	}

	CTipoffCfg(const CTipoffCfg & rv)
	{
		memcpy(params, rv.params, sizeof(TIP_OFFSET_CONFIG));
	}

	CTipoffCfg(const TIP_OFFSET_CONFIG * prv)
	{
		memcpy(params, prv, sizeof(TIP_OFFSET_CONFIG));
	}

	CTipoffCfg(float p[3])
	{
		memcpy(params, p, 3 * sizeof(float));
	}

	operator TIP_OFFSET_CONFIG * () { return (TIP_OFFSET_CONFIG *)this; }
	operator void * () { return (void *)((TIP_OFFSET_CONFIG *)this); }
	bool operator ==  (const float *rv) const 
	{
		return (FP_EQ(params[0], rv[0]) && FP_EQ(params[1], rv[1]) && FP_EQ(params[2], rv[2]));
	}

	CTipoffCfg & operator= (const CTipoffCfg & rv)
	{
		memcpy(params, rv.params, sizeof(TIP_OFFSET_CONFIG));
		return *this;
	}

	CTipoffCfg & operator= (const TIP_OFFSET_CONFIG * prv)
	{
		memcpy(params, prv, sizeof(TIP_OFFSET_CONFIG));
		return *this;
	}

	CTipoffCfg & operator= (const float p[3])
	{
		memcpy(params, p, 3 * sizeof(float));
		return *this;
	}

	void Init()
	{
		memset(params, 0, sizeof(TIP_OFFSET_CONFIG));
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(TIP_OFFSET_CONFIG));
	}

	void Fill(float x = 0, float y = 0, float z = 0)
	{
		Init();
		params[0] = x;
		params[1] = y;
		params[2] = z;

	}

	void Fill( float *p)
	{
		memcpy(params, p, 3 * sizeof(float));
	}

	float & X() { return params[0]; }
	float & Y() { return params[1]; }
	float & Z() { return params[2]; }

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "(x,y,z): (%f, %f, %f)\r\n",
			bTitle ? "TIP_OFFSET_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, params[0], params[1], params[2]);
		s += sz;
	}

	static TIP_OFFSET_CONFIG Default;
};

//typedef struct _INCREMENT_CONFIG
//{
//	uint32_t enabled;
//	float fPosThresh;
//	float fOriThresh;
//}INCREMENT_CONFIG;
class VPCMD_API CIncrementCfg : public INCREMENT_CONFIG
{
public:
	void * TOP() { return &enabled; }
	size_t SZCFG() { return sizeof(INCREMENT_CONFIG); }

	CIncrementCfg()
	{
		Init();
	}

	CIncrementCfg(const CIncrementCfg & rv)
	{
		memcpy(TOP(), &rv.enabled, SZCFG());
	}

	CIncrementCfg(const INCREMENT_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());
	}


	operator INCREMENT_CONFIG * () { return (INCREMENT_CONFIG *)this; }
	operator void * () { return (void *)((INCREMENT_CONFIG *)this); }
	bool operator == (const INCREMENT_CONFIG *prv) const { return FP_EQ(prv->fOriThresh, fOriThresh) && FP_EQ(prv->fPosThresh, fPosThresh); }

	CIncrementCfg(float p[2])
	{
		Init();
		memcpy(&fPosThresh, p, 2 * sizeof(float));
	}


	CIncrementCfg & operator= (const CIncrementCfg & rv)
	{
		memcpy(&fPosThresh, &rv.fPosThresh, sizeof(INCREMENT_CONFIG));
		return *this;
	}

	CIncrementCfg & operator= (const INCREMENT_CONFIG * prv)
	{
		memcpy(&fPosThresh, prv, sizeof(INCREMENT_CONFIG));
		return *this;
	}

	CIncrementCfg & operator= (const float p[2])
	{
		memcpy(&fPosThresh, p, 2 * sizeof(float));
		return *this;
	}
	void Init()
	{
		memset(TOP(), 0, sizeof(INCREMENT_CONFIG));
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(INCREMENT_CONFIG));
	}

	void Fill(bool bEn = false, float posthresh = 0.0f, float orithresh = 0.0f)
	{
		enabled = bEn ? 1 : 0;
		fPosThresh = posthresh;
		fOriThresh = orithresh;
	}

	bool Enabled() { return enabled != 0; }
	bool PosAuto() { return fPosThresh <= 0.0f; }
	bool OriAuto() { return fOriThresh <= 0.0f; }

	void SetPosAuto() { fPosThresh = 0.0f; }
	void SetOriAuto() { fOriThresh = 0.0f; }

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "enabled:  %d %s" "%s"
			"%s" "fPosThresh: %f %s" "%s"
			"%s" "fOriThresh: %f %s\r\n",
			bTitle ? "INCREMENT_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, enabled, Enabled() ? "(Enabled)" : "(Disabled)", RPT_szCRLF,
			RPT_szINDENT, fPosThresh, PosAuto() ? "(AUTO)" : "", RPT_szCRLF,
			RPT_szINDENT, fOriThresh, OriAuto() ? "(AUTO)" : "");
		s += sz;
	}

	static INCREMENT_CONFIG Default;
};

//typedef struct _BORESIGHT_CONFIG
//{
//	float params[4];
//}BORESIGHT_CONFIG;
class VPCMD_API CBoresightCfg : public BORESIGHT_CONFIG
{
public:
	CBoresightCfg( eViperOriUnits eOri = ORI_EULER_DEGREE, bool bResetO = false)
	{
		Init(eOri);
		ResetOrigin(bResetO);// m_arg2_reseto = (bResetO ? 1 : 0);
	}

	CBoresightCfg(const CBoresightCfg & rv)
	{
		memcpy(params, rv.params, sizeof(BORESIGHT_CONFIG));
		m_eOriUnits = rv.m_eOriUnits;
		m_arg2_reseto = rv.m_arg2_reseto;
	}

	CBoresightCfg(const BORESIGHT_CONFIG * prv)
	{
		Init();
		memcpy(params, prv, sizeof(BORESIGHT_CONFIG));
	}

	CBoresightCfg(float p[4])
	{
		Init();
		memcpy(params, p, 4 * sizeof(float));
	}

	operator BORESIGHT_CONFIG * () { return (BORESIGHT_CONFIG *)this; }
	operator void * () { return (void *)((BORESIGHT_CONFIG *)this); }
	bool operator == (const float *rv) const
	{
		return (FP_EQ(params[0], rv[0]) && FP_EQ(params[1], rv[1]) && FP_EQ(params[2], rv[2]) && FP_EQ(params[3], rv[3]));
	}

	CBoresightCfg & operator= (const CBoresightCfg & rv)
	{
		memcpy(params, rv.params, sizeof(BORESIGHT_CONFIG));
		m_eOriUnits = rv.m_eOriUnits;
		m_arg2_reseto = rv.m_arg2_reseto;
		return *this;
	}

	CBoresightCfg & operator= (const BORESIGHT_CONFIG * prv)
	{
		Init();
		memcpy(params, prv, sizeof(BORESIGHT_CONFIG));
		return *this;
	}

	CBoresightCfg & operator= (const float p[4])
	{
		Init();
		memcpy(params, p, 4 * sizeof(float));
		return *this;
	}

	void Init(eViperOriUnits eOri = ORI_EULER_DEGREE)
	{
		m_eOriUnits = eOri;
		//memset(params, 0, sizeof(BORESIGHT_CONFIG));
		memcpy(params, &Default[eOri], sizeof(BORESIGHT_CONFIG));
		m_arg2_reseto = 0;
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(BORESIGHT_CONFIG));
		m_arg2_reseto = 0;
	}

	void Fill(float x = 0, float y = 0, float z = 0, float q4=0, bool bResetO=false)
	{
		params[0] = x;
		params[1] = y;
		params[2] = z;
		params[3] = q4;
		ResetOrigin(bResetO);// m_arg2_reseto = bResetO ? 1 : 0;
	}

	void Fill(float *p)
	{
		memcpy(params, p, 4 * sizeof(float));
	}

	void MakePrintable()
	{

	}

	void Report(std::string & s, uint32_t eU = ORI_MAX, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		// create an ascii string of the units.  
		// If none specified in argument list, use object m_eOriUnits.
		uint32_t leU = (eU == ORI_MAX) ? m_eOriUnits : eU;
		const char * szOriU = CVPcmd::oriunitsstr(leU);

		switch (leU)
		{
		default:
			sprintf_s(sz, 200,
				"%s" "%s"
				"%s" "(%f, %f, %f, %f)\r\n",
				bTitle ? "BORESIGHT_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, params[0], params[1], params[2], params[3]);
			break;
		case ORI_QUATERNION:
			sprintf_s(sz, 200,
				"%s" "%s"
				"%s" "%s (w,x,y,z): (%f, %f, %f, %f)\r\n",
				bTitle ? "BORESIGHT_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, szOriU, params[0], params[1], params[2], params[3]);
			break;
		case ORI_EULER_DEGREE:
		case ORI_EULER_RADIAN:
			sprintf_s(sz, 200,
				"%s" "%s"
				"%s" "%s (x,y,z): (%f, %f, %f)\r\n",
				bTitle ? "BORESIGHT_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
				RPT_szINDENT, szOriU, params[0], params[1], params[2]);
			break;
		}
		s += sz;
	}

	// units attribute is only used for a couple of purposes.  
	// It is for carrying the information around; not really for acting on it.
	// It is not filled by fill operation.
	// It is not copied by operator = or by copy constructor unless from another CBoresightCfg object
	// It is initialized during construction.
	// It is used for the Report operation, if a valid units value is not supplied as an argument.
	// It is used for Enabled state operation
	eViperOriUnits m_eOriUnits;
	eViperOriUnits & Units() { return m_eOriUnits; }
	bool IsQtrn() { return m_eOriUnits == ORI_QUATERNION; }

	// Reset Origin attribute should be applied in arg2 of CMD_BORESIGHT
	uint32_t m_arg2_reseto;
	uint32_t & Arg2() { return m_arg2_reseto; }
	bool ResetOrigin() { return (m_arg2_reseto == 1); }
	void ResetOrigin(bool bResetO) { m_arg2_reseto = bResetO ? 1:0; }

	bool Enabled();


	static BORESIGHT_CONFIG Default[ORI_MAX];
};

//typedef struct _HEMISPHERE_CONFIG
//{
//	struct {
//		uint32_t track_en : 1;
//		uint32_t auto_en : 1;
//		uint32_t res : 30;
//	}bf;
//	float params[3];
//}HEMISPHERE_CONFIG;
class VPCMD_API CHemisphereCfg : public HEMISPHERE_CONFIG
{
public:
	enum eHemPresetVal {
		POS_X, POS_Y, POS_Z, NEG_X, NEG_Y, NEG_Z, CUSTOM,/* AUTO_HEM, TRACK_EN,*/ HEMPRESETVAL_MAX
	};

	CHemisphereCfg()
	{
		Init();
	}

	CHemisphereCfg(const CHemisphereCfg & rv)
	{
		memcpy(&bf, &rv.bf, sizeof(HEMISPHERE_CONFIG));
		eVal = rv.eVal;
	}

	CHemisphereCfg(const HEMISPHERE_CONFIG * prv)
	{
		Init();
		memcpy(&bf, prv, sizeof(HEMISPHERE_CONFIG));
		SetPresetEnum();
	}

	operator HEMISPHERE_CONFIG * () { return (HEMISPHERE_CONFIG *)this; }
	operator void * () { return (void *)((HEMISPHERE_CONFIG *)this); }

	CHemisphereCfg & operator= (const CHemisphereCfg & rv)
	{
		memcpy(&bf, &rv.bf, sizeof(HEMISPHERE_CONFIG));
		SetPresetEnum();
		return *this;
	}

	CHemisphereCfg & operator= (const HEMISPHERE_CONFIG * prv)
	{
		memcpy(params, prv, sizeof(HEMISPHERE_CONFIG));
		SetPresetEnum();
		return *this;
	}
	bool operator ==  (const HEMISPHERE_CONFIG * prv) const
	{
		return (prv->bf.track_en == bf.track_en) && (prv->bf.auto_en == bf.auto_en) && (FP_EQ(params[0], prv->params[0]) && FP_EQ(params[1], prv->params[1]) && FP_EQ(params[2], prv->params[2]) && FP_EQ(params[3], prv->params[3]));
	}

	void Init()
	{
		Fill(POS_X, false);
		//memset(&track_en, 0, sizeof(HEMISPHERE_CONFIG));
		//params[0] = 1.0f;
		//eVal = POS_X;
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(HEMISPHERE_CONFIG));
	}

	size_t Size() { return sizeof(HEMISPHERE_CONFIG); }

	void Fill(float x = 1.0, float y = 0, float z = 0, bool bAutoHem = false, bool bHTrack = false)
	{
		params[0] = x;
		params[1] = y;
		params[2] = z;
		
		bf.auto_en = bAutoHem ? 1 : 0;
		bf.track_en = (bAutoHem || bHTrack) ? 1 : 0;
		SetPresetEnum();
	}

	void Fill(float *p, bool bAutoHem = false, bool bHTrack=false)
	{
		memcpy(params, p, 3 * sizeof(float));
		
		bf.auto_en = bAutoHem ? 1 : 0;
		bf.track_en = (bAutoHem || bHTrack) ? 1 : 0;

		SetPresetEnum();
	}

	void Fill(enum eHemPresetVal e, bool bAutoHem = false, bool bHTrack=false)
	{
		const HEMISPHERE_CONFIG * phc = CVPcmd::hemispherecfg(e);
		memcpy(this, phc, sizeof(HEMISPHERE_CONFIG));
		bf.auto_en = bAutoHem ? 1 : 0;
		bf.track_en = (bAutoHem || bHTrack) ? 1 : 0;
		eVal = e;
	}

	void SetTrackEn(bool b)
	{
		bf.track_en = b ? 1 : 0;
	}
	void SetAutoEn(bool b)
	{
		bf.auto_en = b ? 1 : 0;
	}
	bool TrackEn() { return bf.track_en ? true : false; }
	bool AutoEn() { return bf.auto_en ? true : false; }

	void SetPresetEnum()
	{
		//bool bSet = true;
		float xvec = params[0];
		float yvec = params[1];
		float zvec = params[2];

		if ((xvec != 0) && (yvec == 0.0f) && (zvec == 0.0f))
		{
			eVal = (xvec > 0) ? POS_X : NEG_X;
		}
		else if ((xvec == 0) && (yvec != 0.0f) && (zvec == 0.0f))
		{
			eVal = (yvec > 0) ? POS_Y : NEG_Y;
		}
		else if ((xvec == 0) && (yvec == 0.0f) && (zvec != 0.0f))
		{
			eVal = (zvec > 0) ? POS_Z : NEG_Z;
		}
		//else if ((xvec == 0) && (yvec == 0.0f) && (zvec == 0.0f))
		//{
		//	if (bf.auto_en)
		//		eVal = AUTO_HEM;
		//	else
		//		eVal = TRACK_EN;
		//	bf.track_en = 1;
		//}
		else
		{
			eVal = CUSTOM;

			//if (abs(xvec) > abs(yvec))
			//{
			//	if (abs(xvec) > abs(zvec))
			//		eVal = (xvec > 0) ? CHemisphereCfg::POS_X : CHemisphereCfg::NEG_X;
			//	else
			//		eVal = (zvec > 0) ? CHemisphereCfg::POS_Z : CHemisphereCfg::NEG_Z;
			//}
			//else if (abs(yvec) > abs(xvec))
			//{
			//	if (abs(yvec) > abs(zvec))
			//		eVal = (yvec > 0) ? CHemisphereCfg::POS_Y : CHemisphereCfg::NEG_Y;
			//	else if (abs(zvec) > abs(yvec))
			//		eVal = (zvec > 0) ? CHemisphereCfg::POS_Z : CHemisphereCfg::NEG_Z;
			//
			//}
			//// x == y.  Now what about z
			//else if (abs(zvec) > abs(xvec))
			//{
			//	eVal = (zvec > 0) ? CHemisphereCfg::POS_Z : CHemisphereCfg::NEG_Z;
			//}
			//// x = y and z is < than both.  Don't know what hemisphere that is. Just call it POS_X.
			//else if (abs(zvec) < abs(xvec))
			//{
			//	eVal = (xvec > 0) ? CHemisphereCfg::POS_X : CHemisphereCfg::NEG_X;
			//}
			//// x = y = z.  Are they zero?
			//else if (zvec == 0.0f)
			//{
			//	eVal = CHemisphereCfg::TRACK_EN;
			//	cH.track_en = TRUE;
			//}
			//// if not zero, afu
			//else
			//	eVal = (xvec > 0) ? CHemisphereCfg::POS_X : CHemisphereCfg::NEG_X;
		}

		//if (bf.auto_en)
		//	eVal = AUTO_HEM;
		//else
		//		eVal = TRACK_EN;
		//bf.track_en = 1;

	}

	void MakePrintable()
	{

	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];

		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "%s : %d, %d, %f, %f, %f\r\n",
			bTitle ? "HEMISPHERE_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, CVPcmd::hemispherestr(eVal), bf.track_en, bf.auto_en, params[0], params[1], params[2]);

		s += sz;
	}

	uint32_t eVal;

	//bool TrackingEnabledParams()
	//{
	//	return ((params[0] == 0.0f) && (params[1] == 0.0f) && (params[2] == 0.0f));
	//}

	static HEMISPHERE_CONFIG Default;

};

//typedef struct _SEUID_CONFIG
//{
//	uint32_t seuid;
//}SEUID_CONFIG;
class VPCMD_API CSeuIDCfg : public SEUID_CONFIG
{
public:
	CSeuIDCfg()
	{
		Init();
	}

	CSeuIDCfg(const CSeuIDCfg & rv)
	{
		memcpy(&seuid, &rv.seuid, sizeof(SEUID_CONFIG));
	}

	CSeuIDCfg(const SEUID_CONFIG * prv)
	{
		memcpy(&seuid, prv, sizeof(SEUID_CONFIG));
	}

	CSeuIDCfg(const VIPER_SEUID * prv)
	{
		memcpy(&seuid, prv, sizeof(VIPER_SEUID));
	}

	operator SEUID_CONFIG * () { return (SEUID_CONFIG *)this; }
	operator VIPER_SEUID * () { return &seuid; }
	operator void * () { return (void *)((SEUID_CONFIG *)this); }
	bool operator == (const VIPER_SEUID rv) const { return (rv == seuid); }

	CSeuIDCfg & operator = (const VIPER_SEUID & rv)
	{
		seuid = rv; return *this;
	}

	void Init()
	{
		memset(&seuid, 0, sizeof(SEUID_CONFIG));
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(SEUID_CONFIG));
	}

	void Fill(int32_t id = 0)
	{
		seuid = id;
	}

	void MakePrintable()
	{
	}
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "SEU ID: %d\r\n",
			bTitle ? "SEUID_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, seuid);
		s += sz;
	}

	static SEUID_CONFIG Default;
};

class VPCMD_API CBinaryCfg
{
public:
	BINARY_CONFIG val;

	CBinaryCfg()
	{
		Init();
	}

	CBinaryCfg(const CBinaryCfg & rv)
	{
		val = rv.val;
	}

	CBinaryCfg(const BINARY_CONFIG * prv)
	{
		memcpy(&val, prv, sizeof(BINARY_CONFIG));
	}

	operator BINARY_CONFIG * () { return &val; }
	operator void * () { return (void *)&val; }
	bool operator == (const BINARY_CONFIG *prv) const { return *prv == val; }

	CBinaryCfg & operator = (const BINARY_CONFIG & rv)
	{
		val = rv; return *this;
	}

	operator bool() { return (val != 0); }

	void Init()
	{
		val = 0;
	}
	virtual void InitDefault() { Init(); }

	void Fill(int32_t b = 0)
	{
		val = b;
	}

	virtual void MakePrintable()
	{
	}
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "%d\r\n",
			bTitle ? "BINARY_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val);
		s += sz;
	}
	virtual bool Enabled() { return (val != 0); }


};

class VPCMD_API CSyncModeCfg : public CBinaryCfg
{
public:
	CSyncModeCfg() : CBinaryCfg() {};

	CSyncModeCfg(const CBinaryCfg & rv) : CBinaryCfg(rv) {};

	CSyncModeCfg(const BINARY_CONFIG * prv) : CBinaryCfg(prv) {};

	void InitDefault()
	{
		memcpy(&val, &Default, sizeof(BINARY_CONFIG));
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "SYNC MODE: %d : %s\r\n",
			bTitle ? "SYNC BINARY_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val, 
			val ? "Enabled" : "Disabled");
		s += sz;
	}

	static BINARY_CONFIG Default;

};

class VPCMD_API CDualOutputCfg : public CBinaryCfg
{
public:
	CDualOutputCfg() : CBinaryCfg() {};

	CDualOutputCfg(const CBinaryCfg & rv) : CBinaryCfg(rv) {};

	CDualOutputCfg(const BINARY_CONFIG * prv) : CBinaryCfg(prv) {};

	void InitDefault()
	{
		memcpy(&val, &Default, sizeof(BINARY_CONFIG));
	}
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "DUAL OUTPUT: %d : %s\r\n",
			bTitle ? "DUAL OUTPUT BINARY_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val,
			val ? "Enabled" : "Disabled");
		s += sz;
	}

	static BINARY_CONFIG Default;

};

//typedef struct _ENUM_CONFIG_STRUCT
//{
//	ENUM_CONFIG val;
//}ENUM_CONFIG_STRUCT;
class VPCMD_API CEnumCfg// : public ENUM_CONFIG_STRUCT
{
public:
	ENUM_CONFIG val;

	CEnumCfg()
	{
		Init();
	}

	CEnumCfg(ENUM_CONFIG e)
	{
		val = e;
	}

	CEnumCfg(const CEnumCfg & rv)
	{
		val = rv.val;// memcpy(&val, &rv.val, sizeof(ENUM_CONFIG));
	}

	CEnumCfg(const ENUM_CONFIG * prv)
	{
		memcpy(&val, prv, sizeof(ENUM_CONFIG));
	}

	operator ENUM_CONFIG * () { return &val; }
	operator ENUM_CONFIG () { return val; }
	operator void * () { return (void *)((ENUM_CONFIG *)&val); }
	bool operator == (const ENUM_CONFIG *prv) const { return *prv == val; }

	CEnumCfg & operator = (const ENUM_CONFIG & rv)
	{
		memcpy(&val, &rv, sizeof(ENUM_CONFIG));

		return *this;
	}

	void Init()
	{
		memset(&val, 0, sizeof(ENUM_CONFIG));
	}
	void InitDefault()
	{
		memcpy(&val, &Default, sizeof(ENUM_CONFIG));
	}

	void Fill(int32_t m = 0)
	{
		val = m;
	}

	void MakePrintable()
	{
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "VAL: %d : %d\r\n",
			bTitle ? "ENUM_CONFIG :" : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val, val
		);
		s += sz;
	}

	static ENUM_CONFIG Default;
};

class VPCMD_API CStylusCfg : public CEnumCfg
{
public:
	CStylusCfg() : CEnumCfg() {};
	CStylusCfg(eStylusMode e) : CEnumCfg((ENUM_CONFIG)e) {};
	CStylusCfg(ENUM_CONFIG e) : CEnumCfg(e) {};
	CStylusCfg(ENUM_CONFIG *pe) : CEnumCfg(*pe) {};

	bool IsPT()	{ return val == STYLUS_MODE_POINT; }
	bool IsMrk() { return val == STYLUS_MODE_MARK;  }

	virtual void InitDefault()
	{
		memcpy(&val, &Default, sizeof(ENUM_CONFIG));
	}

	virtual void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "VAL: %d : %s\r\n",
			bTitle ? "STYLUS_CONFIG :" : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val, CVPcmd::stylusmodestr(val)
		);
		s += sz;
	}
	static STYLUS_CONFIG Default;

};

class VPCMD_API CFTTCfg : public CEnumCfg
{
public:
	CFTTCfg() : CEnumCfg() {};
	CFTTCfg(eFTTMode e) : CEnumCfg((ENUM_CONFIG)e) {};

	virtual void InitDefault()
	{
		memcpy(&val, &Default, sizeof(ENUM_CONFIG));
	}

	bool Enabled() { return val != 0; }

	virtual void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "VAL: %d : %s\r\n",
			bTitle ? "FTT_CONFIG :" : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val, CVPcmd::fttstr(val)
		);
		s += sz;
	}
	static FTTMODE_CONFIG Default;
};

//typedef struct _SERIAL_CONFIG
//{
//	union
//	{
//		uint32_t sercfg;
//		struct bf {
//			uint32_t baudrate : 4;  // uses values from eBaud
//			uint32_t parity : 2;	// uses values from eParity
//			uint32_t res : 26;
//		};
//	};
//}SERIAL_CONFIG;
class VPCMD_API CSerialCfg : public SERIAL_CONFIG
{
public:
	CSerialCfg()
	{
		Init();
	}

	CSerialCfg(const CSerialCfg & rv)
	{
		sercfg = rv.sercfg;
	}

	CSerialCfg(const SERIAL_CONFIG * prv)
	{
		sercfg = prv->sercfg;
	}
	CSerialCfg(const uint32_t * prv)
	{
		sercfg = *prv;
	}
	CSerialCfg(eBaud eb, eParity ep)
	{
		Init();
		bf.baudrate = eb;
		bf.parity = ep;
	}
	CSerialCfg(const uint32_t rv)
	{
		Init();
		sercfg = rv;
	}

	operator SERIAL_CONFIG * () { return (SERIAL_CONFIG *)this; }
	operator void * () { return (void *)((SERIAL_CONFIG *)this); }
	bool operator == (const SERIAL_CONFIG *prv) const { return prv->sercfg == sercfg; }
	operator uint32_t () { return sercfg; }

	CSerialCfg & operator = (const SERIAL_CONFIG & rv)
	{
		sercfg = rv.sercfg;

		return *this;
	}

	void Init()
	{
		sercfg = 0;
	}

	void InitDefault()
	{
		memcpy(&sercfg, &Default, sizeof(SERIAL_CONFIG));
	}

	void Fill(uint32_t s)
	{
		sercfg = s;
	}

	void Fill(uint32_t b = E_BR_115200, uint32_t p = E_PARITY_NONE, uint32_t r=0)
	{
		sercfg = 0;
		bf.baudrate = b;
		bf.parity = p;
		bf.res = r;
	}

	void FillBPS(int32_t bps, uint32_t p)
	{
		sercfg;
		bf.baudrate = CVPcmd::bpsbaud(bps);
		bf.parity = p;
	}
	void MakePrintable()
	{
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "sercfg:   0x%04x" "%s"
			"%s" "baudrate: %s" "%s"
			"%s" "parity:   %s" "%s"
			"%s" "res:      0x%04x\r\n",
			bTitle ? "SERIAL_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, sercfg, RPT_szCRLF,
			RPT_szINDENT, CVPcmd::baudstr(bf.baudrate), RPT_szCRLF,
			RPT_szINDENT, CVPcmd::paritystr(bf.parity), RPT_szCRLF,
			RPT_szINDENT, bf.res);
		s += sz;
	}
	void Brief(std::string & s )
	{
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"baud %s, parity %s"
			, CVPcmd::baudvalstr(bf.baudrate)
			, CVPcmd::parityvalstr(bf.parity)
			);
		s += sz;
	}

	int32_t BPS() { return CVPcmd::baudbps(bf.baudrate); }

	static SERIAL_CONFIG Default;
};


//typedef struct _NOISEPROFILE {
//	float    Rq[4][4];			//dq measurment noise 88
//	float    Qq[3][3];			//dq process noise    36
//	float    Rr[3][3];			//dr measurment noise 36
//	float    Qr[3][3];			//dr process noise    36
//}NOISEPROFILE;
//typedef struct _PF_STRUCT {
//	uint32_t     qFil_on;		//quat prediction on/off
//	uint32_t     rFil_on;		//pos prediction on/off
//	uint32_t     predTimeMS		//prediction time ms
//	NOISEPROFILE nprof;
//}PF_CONFIG;
class VPCMD_API CPFNoise : public NOISEPROFILE
{
public:
	CPFNoise()
	{
		InitDefault();
	}

	void InitDefault()
	{
		memset(&Rq[0][0], 0, sizeof(NOISEPROFILE));
	}
	operator NOISEPROFILE * () const { return (NOISEPROFILE *)this; }
	operator void * () { return (void *)((NOISEPROFILE *)this); }

	static NOISEPROFILE Default;
};
class VPCMD_API CPredFiltCfgExt : public PF_CONFIG_EXT
{
public:
	CPredFiltCfgExt()
	{
		Init();
	}

	CPredFiltCfgExt(const CPredFiltCfgExt & rv)
	{
		memcpy(&qFil_on, (PF_CONFIG_EXT*)rv, sizeof(PF_CONFIG_EXT));
	}

	CPredFiltCfgExt(const PF_CONFIG_EXT * prv)
	{
		memcpy(&qFil_on, prv, sizeof(PF_CONFIG_EXT));
	}

	operator PF_CONFIG_EXT * () const { return (PF_CONFIG_EXT *)this; }
	operator void * () { return (void *)((PF_CONFIG_EXT *)this); }

	CPredFiltCfgExt & operator = (const PF_CONFIG_EXT & rv)
	{
		memcpy(&qFil_on, &rv.qFil_on, sizeof(PF_CONFIG_EXT));

		return *this;
	}

	void Init()
	{
		memset(&qFil_on, 0, sizeof(PF_CONFIG_EXT));
		predTimeS = 20.0f / 1000.0f; //default prediction time 20ms
	}

	void InitDefault()
	{
		memcpy(&qFil_on, &Default, sizeof(PF_CONFIG_EXT));
	}

	void Fill(bool bRotPred, bool bPosPred, uint32_t uTimeMS, NOISEPROFILE* noisep=0)
	{
		InitDefault();
		qFil_on = bRotPred ? 1 : 0;
		rFil_on = bPosPred ? 1 : 0;
		if (uTimeMS < 1000)
			predTimeS =  ( uTimeMS /1000.0f);//uTimeMS;//
		if (noisep != 0)
			memcpy(&nprof, noisep, sizeof(NOISEPROFILE));
	}

	void Fill(bool bRotPred, bool bPosPred, float fTimeSec, NOISEPROFILE* noisep = 0)
	{
		InitDefault();
		qFil_on = bRotPred ? 1 : 0;
		rFil_on = bPosPred ? 1 : 0;
		if (fTimeSec < 1.0f)
			predTimeS = fTimeSec;
		if (noisep != 0)
			memcpy(&nprof, noisep, sizeof(NOISEPROFILE));
	}

	bool PosFilterOn() { return rFil_on ? true : false; }
	bool QuatFilterOn() { return qFil_on ? true : false; }

	uint32_t PredTimeMS() { return (uint32_t)(predTimeS*1000.0f); }

	void MakePrintable()
	{
	}
#pragma warning ( disable : 4477 )
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[1000];
		sprintf_s(sz, 1000,
			"%s" "%s"
			"%s" "qFil_on:         %1d" "%s"
			"%s" "pFil_on:         %1d" "%s"
			"%s" "predTime ms, s : %3d,  %0.3f" "%s"
			"%s" "noise profile: %s"
			"%s%s Rq[4][4]: %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %s"
			"%s%s Qq[3][3]: %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %s"
			"%s%s Rr[3][3]: %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %s"
			"%s%s Qr[3][3]: %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g %- 4.2g \r\n"
			, bTitle ? "PF_CONFIG: " : "", bTitle ? RPT_szCRLF : ""
			, RPT_szINDENT, qFil_on, RPT_szCRLF
			, RPT_szINDENT, rFil_on, RPT_szCRLF
			, RPT_szINDENT, (uint32_t)(predTimeS*1000.0f), predTimeS, RPT_szCRLF
			, RPT_szINDENT, RPT_szCRLF
			, RPT_szINDENT, RPT_szINDENT, nprof.Rq[0][0], nprof.Rq[0][1], nprof.Rq[0][2], nprof.Rq[0][3], nprof.Rq[1][0], nprof.Rq[1][1], nprof.Rq[1][2], nprof.Rq[1][3], nprof.Rq[2][0], nprof.Rq[2][1], nprof.Rq[2][2], nprof.Rq[2][3], nprof.Rq[3][0], nprof.Rq[3][1], nprof.Rq[3][2], nprof.Rq[3][3], RPT_szCRLF
			, RPT_szINDENT, RPT_szINDENT, nprof.Qq[0][0], nprof.Qq[0][1], nprof.Qq[0][2], nprof.Qq[1][0], nprof.Qq[1][1], nprof.Qq[1][2], nprof.Qq[2][0], nprof.Qq[2][1], nprof.Qq[2][2], RPT_szCRLF
			, RPT_szINDENT, RPT_szINDENT, nprof.Rr[0][0], nprof.Rr[0][1], nprof.Rr[0][2], nprof.Rr[1][0], nprof.Rr[1][1], nprof.Rr[1][2], nprof.Rr[2][0], nprof.Rr[2][1], nprof.Rr[2][2], RPT_szCRLF
			, RPT_szINDENT, RPT_szINDENT, nprof.Qr[0][0], nprof.Qr[0][1], nprof.Qr[0][2], nprof.Qr[1][0], nprof.Qr[1][1], nprof.Qr[1][2], nprof.Qr[2][0], nprof.Qr[2][1], nprof.Qr[2][2]
		);
		s += sz;
	}
#pragma warning ( default : 4477 )
	static PF_CONFIG_EXT Default;

};
class VPCMD_API CPredFiltCfg : public CPredFiltCfgExt
{
public:
	CPredFiltCfg()
	{
		Init();
	}

	CPredFiltCfg(const CPredFiltCfg & rv)
	{
		memcpy(&qFil_on, (PF_CONFIG*)rv, sizeof(PF_CONFIG));
	}

	CPredFiltCfg(const PF_CONFIG * prv)
	{
		memcpy(&qFil_on, prv, sizeof(PF_CONFIG));
	}
	CPredFiltCfg(const PF_CONFIG_EXT * prv)
	{
		memcpy(&qFil_on, prv, sizeof(PF_CONFIG_EXT));
	}

	operator PF_CONFIG * () const { return (PF_CONFIG *)this; }
	operator void * () { return (void *)((PF_CONFIG *)this); }

	CPredFiltCfg & operator = (const PF_CONFIG & rv)
	{
		memcpy(&qFil_on, &rv.qFil_on, sizeof(PF_CONFIG));

		return *this;
	}

	//void Init()
	//{
	//	memset(&qFil_on, 0, sizeof(PF_CONFIG));
	//	predTimeMS = 100;// 1.0f / 100.0f; //default prediction time 100ms
	//}

	//void InitDefault()
	//{
	//	memcpy(&qFil_on, &Default, sizeof(PF_CONFIG));
	//}

	void Fill(bool bRotPred, bool bPosPred, uint32_t uTimeMS/*, NOISEPROFILE* noisep*/)
	{
		CPredFiltCfgExt::Fill(bRotPred, bPosPred, uTimeMS);
		//InitDefault();
		//qFil_on = bRotPred ? 1 : 0;
		//rFil_on = bPosPred ? 1 : 0;
		//if (uTimeMS < 1000)
		//	predTimeMS = uTimeMS;// (1.0f / uPredTimeMS);
		//if (noisep != 0)
		//	memcpy(&nprof, noisep, sizeof(NOISEPROFILE));
	}

	//bool PosFilterOn() { return rFil_on ? true : false; }
	//bool QuatFilterOn() { return qFil_on ? true : false; }

	void MakePrintable()
	{
	}
#pragma warning ( disable : 4477 )
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[1000];
		sprintf_s(sz, 1000,
			"%s" "%s"
			"%s" "qFil_on:     %1d" "%s"
			"%s" "pFil_on:     %1d" "%s"
			"%s" "predTime ms, s : %3d,  %0.3f" 
			, bTitle ? "PF_CONFIG: " : "", bTitle ? RPT_szCRLF : ""
			, RPT_szINDENT, qFil_on, RPT_szCRLF
			, RPT_szINDENT, rFil_on, RPT_szCRLF
			, RPT_szINDENT, (uint32_t)(predTimeS*1000.0f), predTimeS
		);
		s += sz;
	}
	//#pragma warning ( default : 4477 )
	//	static PF_CONFIG Default;

};

//typedef struct _SRC_SEL_CONFIG {
//	uint32_t uSrcSelMap;
//}SRC_SEL_CONFIG;
class VPCMD_API CSrcSelectCfg : public SRC_SEL_CONFIG
{
public:
	CSrcSelectCfg()
	{
		InitDefault();
	}

	CSrcSelectCfg(const CSrcSelectCfg & rv)
	{
		memcpy(&uSrcSelMap, &rv.uSrcSelMap, sizeof(SRC_SEL_CONFIG));
	}

	CSrcSelectCfg(const SRC_SEL_CONFIG * prv)
	{
		memcpy(&uSrcSelMap, prv, sizeof(SRC_SEL_CONFIG));
	}

	CSrcSelectCfg(const uint32_t * prv)
	{
		memcpy(&uSrcSelMap, prv, sizeof(uint32_t));
	}

	operator SRC_SEL_CONFIG * () { return (SRC_SEL_CONFIG *)this; }
	operator uint32_t * () { return &uSrcSelMap; }
	operator void * () { return (void *)((SRC_SEL_CONFIG *)this); }
	bool operator == (const uint32_t rv) const { return (rv == uSrcSelMap); }

	bool Selected(int src) { return ((uSrcSelMap & (1 << src)) != 0); }

	CSrcSelectCfg & operator = (const uint32_t & rv)
	{
		uSrcSelMap = rv; return *this;
	}

	void Init()
	{
		memset(&uSrcSelMap, 0, sizeof(SRC_SEL_CONFIG));
	}
	void InitDefault()
	{
		memcpy(this, &Default, sizeof(SRC_SEL_CONFIG));
	}

	void Fill(int32_t map = 0)
	{
		uSrcSelMap = map;
	}

	void MakePrintable()
	{
	}
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "SRC_SEL_MAP: 0x%04x\r\n",
			bTitle ? "SRC_SEL_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, uSrcSelMap);
		s += sz;
	}

	static SRC_SEL_CONFIG Default;
};


class VPCMD_API CSnsOrigCfg : public CEnumCfg
{
public:
	CSnsOrigCfg() : CEnumCfg() {};
	CSnsOrigCfg(eSensorOrigin e) : CEnumCfg((ENUM_CONFIG)e) {};
	CSnsOrigCfg(ENUM_CONFIG *pe) : CEnumCfg(*pe) {};
	CSnsOrigCfg(ENUM_CONFIG e) : CEnumCfg(e) {};

	virtual void InitDefault()
	{
		memcpy(&val, &Default, sizeof(ENUM_CONFIG));
	}

	virtual void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "VAL: %d : %s\r\n",
			bTitle ? "SNS_ORIG_CONFIG :" : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, val, CVPcmd::snsorigstr(val)
		);
		s += sz;
	}
	static SNS_ORIG_CONFIG Default;
};

//typedef struct _VIRTUAL_SNS_CONFIG {
//	uint32_t uInput1;
//	uint32_t reserved0;
//	uint32_t reserved1;
//	uint32_t reserved2;
//}VIRTUAL_SNS_CONFIG;

class VPCMD_API CSnsVirtCfg : public VIRTUAL_SNS_CONFIG
{
public:
	uint32_t m_uVirtSns;
	uint32_t & VirtSns() { return m_uVirtSns; }
	void * TOP() const { return (void*)this; }
	size_t SZCFG() { return sizeof(VIRTUAL_SNS_CONFIG); }
	CSnsVirtCfg()
	{
		InitDefault();
	}

	CSnsVirtCfg(const CSnsVirtCfg & rv)
	{
		memcpy(TOP(), rv.TOP(), SZCFG());
	}

	CSnsVirtCfg(const VIRTUAL_SNS_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());
	}

	operator VIRTUAL_SNS_CONFIG * () { return (VIRTUAL_SNS_CONFIG *)this; }
	operator uint32_t * () { return (uint32_t*)TOP(); }
	operator void * () { return (void *)TOP(); }
	bool operator == (const uint32_t rv) const { return (rv == uInput1); }

	CSnsVirtCfg & operator = (const uint32_t & rv)
	{
		uInput1 = rv; return *this;
	}

	void Init()
	{
		memset(TOP(), 0, SZCFG());
		m_uVirtSns = 0;
	}
	void InitDefault()
	{
		memcpy(this, &Default, SZCFG());
		m_uVirtSns = 0;
	}

	void Fill(uint32_t in1, uint32_t in2=0, uint32_t op1=0, uint32_t op2=0)
	{
		Init();
		uInput1 = in1;
		reserved0 = in2;
		reserved1 = op1;
		reserved2 = op2;
	}

	void MakePrintable()
	{
	}
	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		RPT_szINDENT = bTitle ? "   " : "";
		MakePrintable();
		char sz[200];
		sprintf_s(sz, 200,
			"%s" "%s"
			"%s" "Input1: % 2d\r\n",
			bTitle ? "VIRTUAL_SNS_CONFIG: " : "", bTitle ? RPT_szCRLF : "",
			RPT_szINDENT, uInput1);
		s += sz;
	}

	static VIRTUAL_SNS_CONFIG Default;

};

/*
*  @ref CMD_SENSBLOCK_CFG payload.
*  CMD_SENSBLOCK_CFG is reset only.  This structure holds all sensor related settings.
*  Used in conjunction with BLOCK_CFG_PAYLOAD to hold all sensor related settings.
*  byte index | name       | format | description
*  -----------|------------|--------|---------------------------------------------------
*  0-15       | hemisphere | struct | See HEMISPHERE_CONFIG.
*  16-35      | pos_filter | struct | See FILTER_CONFIG.
*  36-55      | ori_filter | struct | See FILTER_CONFIG.
*  56-67      | offset     | struct | See TIP_OFFSET_CONFIG.
*  68-71      | increment  | struct | See INCREMENT_CONFIG.
*  72-75      | src_sel    | struct | See SRC_SEL_CONFIG.
*  76-79      | sns_orig   | struct | See SNS_ORIG_CONFIG.
*/
typedef struct _SENSOR_CONFIG
{
	HEMISPHERE_CONFIG	hemi;			/**  0   set/get/reset,	sensor	HEMISPHERE_CONFIG	p		cfg		sensornum					*/
	FILTER_CONFIG		pos_filter;		/**  1   set/get/reset,	sensor	FILTER_CONFIG		p		cfg		sensornum	eFilterTargets	*/
	FILTER_CONFIG		ori_filter;
	TIP_OFFSET_CONFIG	tip_offset;		/**  2   set/get/reset,	sensor	TIP_OFFSET_CONFIG	p		cfg		sensornum					*/
	INCREMENT_CONFIG	increment;		/**  3   set/get/reset,	sensor	INCREMENT_CONFIG	p		cfg		sensornum				*/
	SRC_SEL_CONFIG		src_sel;		/**  30  set/get/reset,	sensor	SRC_SEL_CONFIG		p		cfg		sensornum						*/
	SNS_ORIG_CONFIG		sns_orig;		/**  31  set/get/reset,	sensor	SNS_ORIG_CONFIG(ENUM)	p		cfg			sensornum						*/
	PF_CONFIG_EXT		pf_cfg;						/**  28  set/get/reset,	SEU		PF_CONFIG			p		cfg							*/
}SENSOR_CONFIG;

// @ref CMD_BLOCK_CFG reference.
/*
*  CMD_BLOCK_CFG is reset only.  This structure holds all settings affected by that command.
*  Use with action @ref CMD_ACTION_RESET to restore factory default configuration to Viper SEU.
*  Follow this with @ref CMD_PERSIST to Persist factory configuration through system reset.
*  byte index | name              | format   | description
*  -----------|-------------------|----------|---------------------------------------------------
*  0-3        | frame_rate        | struct   | See FRAMERATE_PAYLOAD.
*  4-11       | units             | struct   | See UNITS_PAYLOAD.
*  12-139     | src_cfg           | struct[] | See FOR_TRANSLATE_PAYLOAD.
*  140-143    | sync_mode         | struct   | See FOR_ROTATE_PAYLOAD.
*  144-151    | sta_map           | struct   | See FRAMERATE_PAYLOAD.
*  152-155    | stylus_mode       | struct   | See STYLUS_CONFIG
*  156-159    | seuid             | struct   | See VIPER_SEUID
*  160-163    | usb_mode          | struct   | See BINARY_CONFIG
*  164-167    | rs422_mode        | struct   | See SERIAL_CONFIG
*  168+184    | prediction filter | struct   | See PF_CONFIG_EXT
*  168+1152   | sensor_config     | struct[] | See SENSOR_CONFIG
*/
typedef struct _BLOCK_CONFIG
{
	FRAMERATE_CONFIG	frame_rate;					/**  6   set/get/reset,	SEU		FRAMERATE_CONFIG	p		cfg							*/
	UNITS_CONFIG		units;						/**  7   set/get/reset,	SEU		UNITS_CONFIG		p		cfg							*/
	SRC_CONFIG			src_cfg[SOURCES_PER_SEU];	/**  8   set/get/reset,	SEU		SRC_CONFIG			p		cfg							*/
	BINARY_CONFIG		sync_mode;					/**  9   set/get/reset,	SEU		BINARY_CONFIG		p		cfg-binary					*/
	STATION_MAP			sta_map;					/**  10  set/get/reset,	SEU		STATION_MAP					read-only					*/
	STYLUS_CONFIG		stylus_mode;				/**  11  set/get/reset,	sensor	STYLUS_CONFIG(ENUM)	p		cfg							*/
	VIPER_SEUID			seuid;						/**  12  set/get/reset,	SEU		VIPER_SEUID			p		cfg							*/
	BINARY_CONFIG		dualout_mode;				/**  13  set/get/reset,	SEU		BINARY_CONFIG		p		cfg-binary					*/
	SERIAL_CONFIG		ser_cfg;					/**  14  set/get/reset,	SEU		SERIAL_CONFIG		p		cfg							*/
	SENSOR_CONFIG		sensor_config[SENSORS_PER_SEU];
}BLOCK_CONFIG;

//typedef struct _SENSOR_CONFIG
//{
//	HEMISPHERE_CONFIG hemi;			/**  0   set/get/reset,	sensor	HEMISPHERE_CONFIG	p		cfg		sensornum					*/
//	FILTER_CONFIG pos_filter;		/**  1   set/get/reset,	sensor	FILTER_CONFIG		p		cfg		sensornum	eFilterTargets	*/
//	FILTER_CONFIG ori_filter;
//	TIP_OFFSET_CONFIG tip_offset;	/**  2   set/get/reset,	sensor	TIP_OFFSET_CONFIG	p		cfg		sensornum					*/
//	INCREMENT_CONFIG increment;		/**  3   set/get/reset,	sensor	INCREMENT_CONFIG	p		cfg			sensornum				*/
//}SENSOR_CONFIG;
class VPCMD_API CSensBlockCfg : SENSOR_CONFIG
{
public:
	void * TOP() { return &hemi; }
	size_t SZCFG() { return sizeof(SENSOR_CONFIG); }

	CSensBlockCfg() { Init(); }
	CSensBlockCfg(const CSensBlockCfg & rv)
	{
		memcpy(TOP(), &rv.hemi, SZCFG());
	}

	CSensBlockCfg( SENSOR_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());
	}

	operator SENSOR_CONFIG * () { return (SENSOR_CONFIG *)this; }
	operator void * () { return (void *)((SENSOR_CONFIG *)this); }
	CSensBlockCfg & operator = (const SENSOR_CONFIG & rv)
	{
		memcpy(TOP(), &rv, SZCFG());

		return *this;
	}
	CSensBlockCfg & operator = (const SENSOR_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());

		return *this;
	}
	void Init()
	{
		memset(TOP(), 0, SZCFG());
	}
	void InitDefault()
	{
		memcpy(TOP(), &Default, SZCFG());
	}

	void Fill(const SENSOR_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());
	}

	void MakePrintable()
	{
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		MakePrintable();
		char sz[200];
		if (bTitle)
		{
			sprintf_s(sz, 200, "SENSOR_CONFIG: " "%s", RPT_szCRLF);
			s += sz;
		}
		CHemisphereCfg(PHemisphere()).Report(s, bTitle, bFlat);
		if (bTitle)
		{
			s += CVPcmd::filtertarstr(FILTER_TRGT_POS); s += " ";
		}
		CFilterCfg(PFilterPos()).Report(s, bTitle, bFlat);
		if (bTitle)
		{
			s += CVPcmd::filtertarstr(FILTER_TRGT_ORI); s += " ";
		}
		CFilterCfg(PFilterOri()).Report(s, bTitle, bFlat);
		CTipoffCfg(PTipoff()).Report(s, bTitle, bFlat);
		//CStylusCfg(PStylusMode()).Report(s, bTitle, bFlat);
		//CBoresightCfg(Boresight()).Report(s, ORI_MAX, bTitle, bFlat);
		CIncrementCfg(PIncrement()).Report(s, bTitle, bFlat);
		CSrcSelectCfg(PSrcSelect()).Report(s, bTitle, bFlat);
		CSnsOrigCfg(PSnsOrigin()).Report(s, bTitle, bFlat);
		CPredFiltCfgExt(PPFExt()).Report(s, bTitle, bFlat);
	}


	bool operator == (const SENSOR_CONFIG *prv)
	{
		return 
			(CHemisphereCfg(PHemisphere()) == &prv->hemi) &&
			(CFilterCfg(PFilterOri()) == CFilterCfg(&prv->ori_filter)) &&
			(CFilterCfg(PFilterPos()) == CFilterCfg(&prv->pos_filter)) &&
			(CTipoffCfg(PTipoff()) == &prv->tip_offset) &&
			//(CStylusCfg(PStylusMode()) == &prv->stylus_mode) &&
			(CIncrementCfg(PIncrement()) == &prv->increment);

	}

	HEMISPHERE_CONFIG *	PHemisphere()	{ return (&hemi);		}
	FILTER_CONFIG *		PFilterOri()	{ return (&ori_filter);	}
	FILTER_CONFIG *		PFilterPos()	{ return (&pos_filter);	}
	TIP_OFFSET_CONFIG *	PTipoff()		{ return (&tip_offset);	}
	//STYLUS_CONFIG *		PStylusMode()	{ return (&stylus_mode); }
	//BORESIGHT_CONFIG *	Boresight()		{ return (&boresight);	}
	INCREMENT_CONFIG *	PIncrement()	{ return (&increment);	}
	SRC_SEL_CONFIG *	PSrcSelect()	{ return (&src_sel);	}
	SNS_ORIG_CONFIG *	PSnsOrigin()	{ return (&sns_orig);	}
	PF_CONFIG_EXT *		PPFExt()		{ return (PF_CONFIG_EXT*)(&pf_cfg); }

	void SetHemisphere	(HEMISPHERE_CONFIG *	rv) { memcpy(PHemisphere(),	rv, sizeof(HEMISPHERE_CONFIG));	}
	void SetFilterOri	(FILTER_CONFIG *		rv) { memcpy(PFilterOri(),	rv, sizeof(FILTER_CONFIG));		}
	void SetFilterPos	(FILTER_CONFIG *		rv) { memcpy(PFilterPos(),	rv, sizeof(FILTER_CONFIG));		}
	void SetTipoff		(TIP_OFFSET_CONFIG *	rv) { memcpy(PTipoff(),		rv, sizeof(TIP_OFFSET_CONFIG));	}
	//void SetStylusMode	(STYLUS_CONFIG *		rv) { memcpy(PStylusMode(), rv, sizeof(STYLUS_CONFIG));		}
	void SetIncrement	(INCREMENT_CONFIG *		rv) { memcpy(PIncrement(),	rv, sizeof(INCREMENT_CONFIG));	}
	void SetSrcSelect	(SRC_SEL_CONFIG *		rv) { memcpy(PSrcSelect(), rv, sizeof(SRC_SEL_CONFIG)); }
	void SetSnsOrigin	(SNS_ORIG_CONFIG *		rv) { memcpy(PSnsOrigin(), rv, sizeof(SNS_ORIG_CONFIG)); }
	void SetPredFilt	(PF_CONFIG *			rv) { memcpy(PPFExt(),     rv, sizeof(PF_CONFIG)); }
	void SetPredFiltExt	(PF_CONFIG_EXT*			rv) { memcpy(PPFExt(),     rv, sizeof(PF_CONFIG_EXT)); }

	static SENSOR_CONFIG Default;

};

// !! This class is used only for accessing a SENSOR_CONFIG block inside of a SEU BLOCK_CONFIG.
class VPCMD_API CBlockCfgSens 
{
protected:
	SENSOR_CONFIG *psens;
public:
	void * TOP() const { return (void *)psens; }
	size_t SZCFG() { return sizeof(SENSOR_CONFIG); }

	//CBlockCfgSens() { Init(); }
	//CBlockCfgSens(const CBlockCfgSens & rv)
	//{
	//	memcpy(TOP(), &rv.TOP(), SZCFG());
	//}

	CBlockCfgSens(SENSOR_CONFIG * prv=0)
	{
		//memcpy(TOP(), prv, SZCFG());
		//this = prv;
		psens = prv;
	}

	operator SENSOR_CONFIG * () { return psens; }
	operator void * () { return (void *)(psens); }
	CBlockCfgSens & operator = (const SENSOR_CONFIG & rv)
	{
		if (TOP())
			memcpy(TOP(), &rv, SZCFG());

		return *this;
	}
	CBlockCfgSens & operator = (const CBlockCfgSens & rv)
	{
		if (TOP() && rv.TOP())
			memcpy(TOP(), rv.TOP(), SZCFG());

		return *this;
	}

	void Init()
	{
		if (TOP())
			memset(TOP(), 0, SZCFG());
	}
	void InitDefault()
	{
		if (TOP())
			memcpy(TOP(), &Default, SZCFG());
	}

	void Fill(const SENSOR_CONFIG * prv)
	{
		if (TOP())
			memcpy(TOP(), prv, SZCFG());
	}

	void MakePrintable()
	{
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		RPT_szCRLF = bFlat ? "" : RPT_CRLF;
		MakePrintable();
		char sz[200];
		if (bTitle)
		{
			sprintf_s(sz, 200, "SENSOR_CONFIG: " "%s", RPT_szCRLF);
			s += sz;
		}
		if (psens == 0)
		{
			s += "NULL PTR";
			return;
		}
		CHemisphereCfg(Hemisphere()).Report(s, bTitle, bFlat);
		if (bTitle)
		{
			s += CVPcmd::filtertarstr(FILTER_TRGT_POS); s += " ";
		}
		CFilterCfg(FilterPos()).Report(s, bTitle, bFlat);
		if (bTitle)
		{
			s += CVPcmd::filtertarstr(FILTER_TRGT_ORI); s += " ";
		}
		CFilterCfg(FilterOri()).Report(s, bTitle, bFlat);
		CTipoffCfg(Tipoff()).Report(s, bTitle, bFlat);
		//CStylusCfg(StylusMode()).Report(s, bTitle, bFlat);
		CIncrementCfg(Increment()).Report(s, bTitle, bFlat);
	}


	bool operator == (const SENSOR_CONFIG *prv)
	{
		return
			(CHemisphereCfg(Hemisphere()) == &prv->hemi) &&
			(CFilterCfg(FilterOri()) == CFilterCfg(&prv->ori_filter)) &&
			(CFilterCfg(FilterPos()) == CFilterCfg(&prv->pos_filter)) &&
			(CTipoffCfg(Tipoff()) == &prv->tip_offset) &&
			//(CStylusCfg(StylusMode()) == &prv->stylus_mode) &&
			(CIncrementCfg(Increment()) == &prv->increment);

	}

	HEMISPHERE_CONFIG *	Hemisphere()	{ return psens ? &psens->hemi       : nullptr; }
	FILTER_CONFIG *		FilterOri()		{ return psens ? &psens->ori_filter : nullptr; }
	FILTER_CONFIG *		FilterPos()		{ return psens ? &psens->pos_filter : nullptr; }
	TIP_OFFSET_CONFIG *	Tipoff()		{ return psens ? &psens->tip_offset : nullptr; }
	//STYLUS_CONFIG *		StylusMode()	{ return psens ? &psens->stylus_mode  : nullptr; }
	INCREMENT_CONFIG *	Increment()		{ return psens ? &psens->increment  : nullptr; }

	void SetHemisphere(HEMISPHERE_CONFIG *	rv)     { memcpy(Hemisphere(), rv, sizeof(HEMISPHERE_CONFIG)); }
	void SetFilterOri(FILTER_CONFIG *		rv)     { memcpy(FilterOri(), rv, sizeof(FILTER_CONFIG)); }
	void SetFilterPos(FILTER_CONFIG *		rv)     { memcpy(FilterPos(), rv, sizeof(FILTER_CONFIG)); }
	void SetTipoff(TIP_OFFSET_CONFIG *	rv)         { memcpy(Tipoff(), rv, sizeof(TIP_OFFSET_CONFIG)); }
	//void SetStylusMode(STYLUS_CONFIG *		rv)		{ memcpy(StylusMode(), rv, sizeof(STYLUS_CONFIG)); }
	void SetIncrement(INCREMENT_CONFIG *		rv) { memcpy(Increment(), rv, sizeof(INCREMENT_CONFIG)); }

	static SENSOR_CONFIG Default;

};

//typedef struct _BLOCK_CONFIG
//{
//		FRAMERATE_CONFIG	frame_rate;					/**  6   set/get/reset,	SEU		FRAMERATE_CONFIG	p		cfg							*/
//		UNITS_CONFIG		units;						/**  7   set/get/reset,	SEU		UNITS_CONFIG		p		cfg							*/
//		SRC_CONFIG			src_cfg[SOURCES_PER_SEU];	/**  8   set/get/reset,	SEU		SRC_CONFIG			p		cfg							*/
//		BINARY_CONFIG		sync_mode;					/**  9   set/get/reset,	SEU		BINARY_CONFIG		p		cfg-binary					*/
//		STATION_MAP			sta_map;					/**  10  set/get/reset,	SEU		STATION_MAP					read-only					*/
//		STYLUS_CONFIG		stylus_mode;				/**  11  set/get/reset,	sensor	STYLUS_CONFIG(ENUM)	p		cfg							*/
//		VIPER_SEUID			seuid;						/**  12  set/get/reset,	SEU		VIPER_SEUID			p		cfg							*/
//		BINARY_CONFIG		dualout_mode;				/**  13  set/get/reset,	SEU		BINARY_CONFIG		p		cfg-binary					*/
//		SERIAL_CONFIG		ser_cfg;					/**  14  set/get/reset,	SEU		SERIAL_CONFIG		p		cfg							*/
//		PF_CONFIG_EXT		pf_cfg;						/**  28  set/get/reset,	SEU		PF_CONFIG			p		cfg							*/
//		SENSOR_CONFIG		sensor_config[SENSORS_PER_SEU];
//}BLOCK_CONFIG;
class VPCMD_API CBlockCfg : public BLOCK_CONFIG
{
public:
	void * TOP() { return &frame_rate; }
	size_t SZCFG() { return sizeof(BLOCK_CONFIG); }

	CBlockCfg() { Init(); }
	CBlockCfg(const CBlockCfg & rv)
	{
		memcpy(TOP(), &rv.frame_rate, SZCFG());
	}

	CBlockCfg(const BLOCK_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());
	}

	operator BLOCK_CONFIG * () { return (BLOCK_CONFIG *)this; }
	operator void * () { return (void *)((BLOCK_CONFIG *)this); }
	CBlockCfg & operator = (const BLOCK_CONFIG & rv)
	{
		memcpy(TOP(), &rv, SZCFG());

		return *this;
	}

	//CBlockCfgSens & operator[] (int s) { return CBlockCfgSens(PSensorBlock(s)); }
	//CBlockCfgSens & operator[] (int s) { return SensorBlock(s); }

	void Init()
	{
		memset(TOP(), 0, SZCFG());
	}
	void InitDefault()
	{
		memcpy(TOP(), &Default, sizeof(BLOCK_CONFIG));
	}

	void Fill(const BLOCK_CONFIG * prv)
	{
		memcpy(TOP(), prv, SZCFG());
	}

	void MakePrintable()
	{
	}

	void Report(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		ReportSEU(s, bTitle, bFlat);
		ReportSensors(s, false, bTitle, bFlat);
	}

	void ReportSEU(std::string & s, bool bTitle = false, bool bFlat = false)
	{
		MakePrintable();
		//char sz[200];
		if (bTitle)
		{
			s += "BLOCK_CONFIG: ";
			s += (bFlat) ? "" : RPT_CRLF;
		}
		CFrameRateCfg(&frame_rate).Report(s, bTitle, bFlat);
		CUnitsCfg(&units).Report(s, bTitle, bFlat);
		//CSrcRotCfg(&srcrot).Report(s, bTitle), bFlat);
		s += "src 0 ";
		CSrcCfg(&src_cfg[0]).Report(s, bTitle, bFlat);
		s += "src 1 ";
		CSrcCfg(&src_cfg[1]).Report(s, bTitle, bFlat);
		s += "src 2 ";
		CSrcCfg(&src_cfg[2]).Report(s, bTitle, bFlat);
		s += "src 3 ";
		CSrcCfg(&src_cfg[3]).Report(s, bTitle, bFlat);
		CStationMap(&sta_map).Report(s, bTitle, bFlat);
		CStylusCfg(PStylusMode()).Report(s, bTitle, bFlat);
		CSyncModeCfg(&sync_mode).Report(s, bTitle, bFlat);
		CSeuIDCfg(&seuid).Report(s, bTitle, bFlat);
		CDualOutputCfg(&dualout_mode).Report(s, bTitle, bFlat);
		CSerialCfg(&ser_cfg).Report(s, bTitle, bFlat);

	}
	void ReportSensors(std::string & s, bool bDetOnly=false, bool bTitle = false, bool bFlat = false)
	{
		MakePrintable();
		char sz[200];
		for (int i = 0; i < SENSORS_PER_SEU; i++)
		{
			if (bDetOnly && !(sta_map.bf.sensor_map & (1 << i)))
			{}
			//else if (bActOnly && !(sta_map.station_map & (1 << i)))
			//{}
			else
			{	
				if (bTitle)
				{
					sprintf_s(sz, 200, "SENSOR_CONFIG[%d]:\r\n", i);
					s += sz;
				}
				CHemisphereCfg(PHemisphere(i)).Report(s, bTitle, bFlat);
				if (bTitle)
				{
					s += CVPcmd::filtertarstr(FILTER_TRGT_POS); s += " ";
				}
				CFilterCfg(PFilterPos(i)).Report(s, bTitle, bFlat);
				if (bTitle)
				{
					s += CVPcmd::filtertarstr(FILTER_TRGT_ORI); s += " ";
				}
				CFilterCfg(PFilterOri(i)).Report(s, bTitle, bFlat);
				CTipoffCfg(PTipoff(i)).Report(s, bTitle, bFlat);
				//CBoresightCfg(PBoresight(i)).Report(s, ORI_MAX, bTitle, bFlat);
				CIncrementCfg(PIncrement(i)).Report(s, bTitle, bFlat);
			}
		}
	}

	bool operator == (const BLOCK_CONFIG *prv)
	{
		return (CFrameRateCfg(PFrameRate()) == CFrameRateCfg(&prv->frame_rate)) &&
			(CUnitsCfg(PUnits()) == &prv->units) &&
			(CSrcCfg(PSrcCfg(0)) == &prv->src_cfg[0]) &&
			(CSrcCfg(PSrcCfg(1)) == &prv->src_cfg[1]) &&
			(CSrcCfg(PSrcCfg(2)) == &prv->src_cfg[2]) &&
			(CSrcCfg(PSrcCfg(3)) == &prv->src_cfg[3]) &&
			(CStationMap(PStationMap()) == &prv->sta_map) &&
			(CStylusCfg(PStylusMode()) == &prv->stylus_mode) &&
			(CSyncModeCfg(PSyncMode()) == &prv->sync_mode) &&
			(CSeuIDCfg(PSeuID()) == prv->seuid) &&
			(CDualOutputCfg(PDualOutMode()) == &prv->dualout_mode) &&
			(PSerial()->sercfg == prv->ser_cfg.sercfg);
				
	}

	FRAMERATE_CONFIG *		PFrameRate()   { return &frame_rate; }
	UNITS_CONFIG *			PUnits()       { return (&units); }
	//SRCROT_CONFIG *			PSrcRot()     { return (&srcrot); }
	//SRC_CONFIG *			PSrcCfg()      { return src_cfg; }
	STATION_MAP *			PStationMap()  { return (&sta_map); }
	STYLUS_CONFIG *			PStylusMode()  { return (&stylus_mode); }
	BINARY_CONFIG *			PSyncMode()    { return (&sync_mode); }
	VIPER_SEUID *			PSeuID()       { return (&seuid); }
	BINARY_CONFIG *			PDualOutMode() { return (&dualout_mode); }
	SERIAL_CONFIG *			PSerial()      { return (SERIAL_CONFIG*)(&ser_cfg); }

	SRC_CONFIG *		PSrcCfg		(int s)  { return &src_cfg[s]; }
	HEMISPHERE_CONFIG *	PHemisphere	(int s)  { return (&sensor_config[s].hemi);		  }
	FILTER_CONFIG *		PFilterOri	(int s)  { return (&sensor_config[s].ori_filter); }
	FILTER_CONFIG *		PFilterPos	(int s)  { return (&sensor_config[s].pos_filter); }
	TIP_OFFSET_CONFIG *	PTipoff		(int s)  { return (&sensor_config[s].tip_offset); }
	//STYLUS_CONFIG *		PStylusMode (int s)  { return (&sensor_config[s].stylus_mode); }
	INCREMENT_CONFIG *	PIncrement	(int s)  { return (&sensor_config[s].increment);  }
	SENSOR_CONFIG *		PSensorBlock	(int s)  { return (&sensor_config[s]);  }

	void SetFrameRate	(FRAMERATE_CONFIG *		rv)	{ memcpy(PFrameRate(),	rv, sizeof(FRAMERATE_CONFIG));	}
	void SetUnits		(UNITS_CONFIG *			rv)	{ memcpy(PUnits(),		rv, sizeof(UNITS_CONFIG));		}
	void SetSrcCfg		(int s, SRC_CONFIG *    rv)	{ memcpy(PSrcCfg(s),	rv, sizeof(SRC_CONFIG));		}
	void SetStationMap	(STATION_MAP *	rv)			{ memcpy(PStationMap(),	rv, sizeof(STATION_MAP));}
	void SetStylusMode	(STYLUS_CONFIG *		rv)	{ memcpy(PStylusMode(), rv, sizeof(STYLUS_CONFIG));		}
	void SetSyncMode	(BINARY_CONFIG *		rv)	{ memcpy(PSyncMode(),	rv, sizeof(BINARY_CONFIG));		}
	void SetSeuID		(VIPER_SEUID *			rv)	{ memcpy(PSeuID(),		rv, sizeof(VIPER_SEUID));		}
	void SetDualOutMode	(BINARY_CONFIG *		rv)	{ memcpy(PDualOutMode(),rv, sizeof(BINARY_CONFIG));		}
	void SetSerial		(SERIAL_CONFIG *		rv)	{ memcpy(PSerial(),		rv, sizeof(SERIAL_CONFIG));		}

	void SetHemisphere	(int s, HEMISPHERE_CONFIG *	rv) { memcpy(PHemisphere(s),	rv, sizeof(HEMISPHERE_CONFIG)); }
	void SetFilterOri	(int s, FILTER_CONFIG *		rv) { memcpy(PFilterOri(s),	rv, sizeof(FILTER_CONFIG));		}
	void SetFilterPos	(int s, FILTER_CONFIG *		rv) { memcpy(PFilterPos(s),	rv, sizeof(FILTER_CONFIG));		}
	void SetTipoff		(int s, TIP_OFFSET_CONFIG *	rv) { memcpy(PTipoff(s), rv, sizeof(TIP_OFFSET_CONFIG)); }
	//void SetStylusMode	(int s, STYLUS_CONFIG *	rv)		{ memcpy(PStylusMode(s), rv, sizeof(STYLUS_CONFIG)); }
	void SetIncrement	(int s, INCREMENT_CONFIG *	rv) { memcpy(PIncrement(s),	rv, sizeof(INCREMENT_CONFIG));	}

	static BLOCK_CONFIG Default;

};

