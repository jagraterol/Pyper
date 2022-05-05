// VSS $Header: /Viper/VPCmdIF/VPCmdIF/VPif.cpp 41    3/20/20 6:35p Suzanne $

#include "VPif.h"
#include <vector>
//#include "VPtypesUT.h"

//#ifndef CLAMP
//#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
//#endif

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

char *RPT_szCRLF = RPT_CRLF;
char *RPT_szINDENT = "";


//inline bool CFrameInfo::IsCmd() const
//{
//	if (IsNull())
//		return false;
//	else if (((VP_FRAME_HDR*)pF)->preamble == VIPER_CMD_PREAMBLE)
//		return true;
//	//else if (((VP_FRAME_HDR*)pF)->preamble == TAG_CMD_PREAMBLE) 
//	//	return true;
//	else
//		return false;
//}
//inline bool CFrameInfo::IsPno() const
//{
//	if (IsNull())
//		return false;
//	else if (((VP_FRAME_HDR*)pF)->preamble == VIPER_PNO_PREAMBLE)
//		return true;
//	//else if (((VP_FRAME_HDR*)pF)->preamble == TAG_PNO_PREAMBLE)
//	//	return true;
//	else
//		return false;
//}

typedef enum
{
	CMDTYPE_CFG
	, CMDTYPE_BINCFG
	, CMDTYPE_EXEC
	, CMDTYPE_PNO
	, CMDTYPE_RO

	, CMDTYPE_MAX
}eCmdType;
typedef enum
{
	SCOPE_SENSOR
	,SCOPE_SEU

	,SCOPE_MAX
}eCmdScope;
typedef enum
{
	CFG_RUNTIME
	,CFG_PERSIST
	,CFG_NA

	,CFG_MAX
}eCfgPersistence;

#define SGR_MAP(s,g,r)  ((s << CMD_ACTION_SET) | (g << CMD_ACTION_GET) | (r << CMD_ACTION_RESET))

#define SET_GET			SGR_MAP(1,1,0)
#define SET_GET_RESET   SGR_MAP(1,1,1)
#define SET_RESET		SGR_MAP(1,0,1)
#define GET_RESET		SGR_MAP(0,1,1)
#define SETONLY			SGR_MAP(1,0,0)
#define GETONLY			SGR_MAP(0,1,0)
#define RESETONLY		SGR_MAP(0,0,1)

#define ACT_MAP(a)(1 << a)

#define NO_TAG_EQUIV  ((uint32_t)-1)

// the following useful for experimental/new devices 'tag'
//typedef struct _cmdinfo_tag
//{
//	eTagCmds			cmd;
//	eViperCmds			vpcmd_equiv;
//	char *				szcmd;
//	uint8_t				actionmap;
//	eCmdScope			cmdscope;
//	size_t				payload_size;
//	eCfgPersistence		cfgpersistence;
//	eCmdType			cmdtype;
//
//}cmdinfo_tag;
//cmdinfo_tag g_cmdinfo_tag[(int)G4P_TOTAL_COMMANDS +1] =
//{ TAG_TOTAL_COMMANDS, CMD_MAX, "TAG_CMD_MAX", 0, SCOPE_MAX, 0, CFG_MAX, CMDTYPE_MAX };


typedef struct _cmdinfo
{
	eViperCmds			cmd;
	char *				szcmd;
	uint8_t				actionmap;
	eCmdScope			cmdscope;
	size_t				payload_size;
	eCfgPersistence		cfgpersistence;
	eCmdType			cmdtype;
	uint32_t			tag_equiv;

}cmdinfo;
cmdinfo g_cmdinfo[(int)CMD_MAX+1] =
{	CMD_MAX, "CMD_MAX", 0, SCOPE_MAX, 0, CFG_MAX, CMDTYPE_MAX, (uint32_t)-1 };

typedef struct _actinfo
{
	eCmdActions			act;
	char *				szact;

}actinfo;
actinfo g_actinfo[CMD_ACTION_MAX+1] =
{	CMD_ACTION_MAX, "CMD_ACTION_MAX" };

typedef struct _posunitsinfo
{
	eViperPosUnits	eu;
	char *			szu;
	char *			szu_pub;
	char *			szu_brief;
}posunitsinfo;
posunitsinfo g_posuinfo[POS_MAX+1] =
{ 	POS_MAX, "POS_MAX", "", "" };

typedef struct _oriunitsinfo
{
	eViperOriUnits	eu;
	char *			szu;
	char *			szu_pub;
	char *			szu_brief;
}oriunitsinfo;
oriunitsinfo g_oriuinfo[ORI_MAX+1] =
{	ORI_MAX, "ORI_MAX", "", "" };

typedef struct _frateinfo
{
	eViperFrameRate efr;
	char *			szfr;
}framerateinfo;
framerateinfo g_frateinfo[FR_MAX + 1] =
{	FR_MAX, "FR_MAX" };

typedef struct _filterlevinfo
{
	eViperFilterLevel	el;
	char *				szl;
}filterlevinfo;
filterlevinfo g_flevinfo[FILTER_LVL_MAX + 1] =
{ FILTER_LVL_MAX, "FILTER_LVL_MAX"};

typedef struct _filtertarinfo
{
	eViperFilterTargets et;
	char *				szt;
}filtertarinfo;
filtertarinfo g_ftarinfo[FILTER_TRGT_MAX + 1] =
{ FILTER_TRGT_MAX, "FILTER_TRGT_MAX" };

typedef struct _fttinfo
{
	eFTTMode	eftt;
	char *		szftt;
}fttinfo;
fttinfo g_fttinfo[FTT_MODE_MAX + 1] =
{ FTT_MODE_MAX, "FTT_MODE_MAX" };

typedef struct _hemisphereinfo
{
	uint32_t et;
	char * szt;
	HEMISPHERE_CONFIG cfg;
}hemisphereinfo;
hemisphereinfo g_heminfo[CHemisphereCfg::HEMPRESETVAL_MAX + 1] =
{ CHemisphereCfg::HEMPRESETVAL_MAX, "HEMPRESETVAL_MAX", {0, 0, 0, 0.0f, 0.0f, 0.0f} };

typedef struct _vpheminfo
{
	eViperHemisphere ehem;
	char *   szhem;
}vpheminfo;
vpheminfo g_vpheminfo[E_VP_HEM_MAX + 1] =
{ E_VP_HEM_MAX, "E_VP_HEM_MAX" };

typedef struct _baudinfo
{
	eBaud eb;
	char * szeb;
	char * szval;
	int bps;
}baudinfo;
baudinfo g_baudinfo[E_BR_MAX + 1] =
{ E_BR_MAX, "E_BR_MAX", "undef", 0 };

typedef struct _parityinfo
{
	eParity ep;
	char * szep;
	char * szval;
}parityinfo;
parityinfo g_parityinfo[E_PARITY_MAX + 1] =
{ E_PARITY_MAX, "E_PARITY_MAX", "undef" };

typedef struct _bitcodeinfo
{
	eBITcode ep;
	char * szep;
	char * szval;
}bitcodeinfo;
bitcodeinfo g_bitcodeinfo[E_BITERR_MAX + 1] =
{ E_BITERR_MAX, "E_BITERR_MAX", "undef" };

typedef struct _stylusinfo
{
	eStylusMode e;
	char *sz;
}stylusinfo;
stylusinfo g_stylusinfo[STYLUS_MODE_MAX + 1] =
{ STYLUS_MODE_MAX, "STYLUS_MODE_MAX" };

typedef struct _snsoriginfo
{
	eSensorOrigin e;
	char *sz;
}snsoriginfo;
snsoriginfo g_snsoriginfo[SNS_ORIG_MAX + 1] =
{ SNS_ORIG_MAX, "SNS_ORIG_MAX" };

//void InitCmdInfo_tag()
//{
//	if (g_cmdinfo_tag[0].cmd != TAG_TOTAL_COMMANDS)
//		return;
//
//	for (int i = 0; i < TAG_TOTAL_COMMANDS; i++)
//	{
//		switch (i)
//		{
//			/**				cmd					cmdstr				actions			scope			paysize						persistent		cmdtyp	arg1/scope?		arg2/qualifier?	*/
//			/**-------------------------------------------------------------------------------------------------------------*/
//		default:
//			g_cmdinfo_g4p[i] = { TAG_TOTAL_COMMANDS, CMD_MAX, "CMD_UNDEFINED",	0,	SCOPE_MAX, 0, CFG_MAX, CMDTYPE_MAX };
//			break;
//
//		};
//	}
//
//}

void CVPcmd::InitCmdInfo()
{
	//InitCmdInfo_tag();

	// Don't initialize it if it is already filled.
	if (g_cmdinfo[0].cmd != CMD_MAX)
		return;

	for (int i = 0; i <= CMD_MAX; i++)
	{
		switch(i)
		{		
			/**				cmd					cmdstr				actions			scope			paysize						persistent		cmdtyp	arg1/scope?		arg2/qualifier?	*/
			/**-------------------------------------------------------------------------------------------------------------*/
		default:
			g_cmdinfo[i] = { CMD_MAX,			"CMD_UNDEFINED",	0,	SCOPE_MAX, 0, CFG_MAX, CMDTYPE_MAX, (uint32_t)-1 };
			break;
		case CMD_SENSOR_BLOCKCFG:
			g_cmdinfo[i] = { CMD_SENSOR_BLOCKCFG,"CMD_SENSOR_BLOCKCFG", RESETONLY, SCOPE_SENSOR,sizeof(SENSOR_CONFIG),			CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SOURCE_CFG:
			g_cmdinfo[i] = { CMD_SOURCE_CFG,	"CMD_SOURCE_CFG",	SET_GET_RESET,	SCOPE_SEU,		sizeof(SRC_CONFIG),			CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_HEMISPHERE:
			g_cmdinfo[i] = { CMD_HEMISPHERE,	"CMD_HEMISPHERE",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(HEMISPHERE_CONFIG),	CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_FILTER:
			g_cmdinfo[i] = { CMD_FILTER,		"CMD_FILTER",		SET_GET_RESET,	SCOPE_SENSOR,	sizeof(FILTER_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_TIP_OFFSET:
			g_cmdinfo[i] = { CMD_TIP_OFFSET,	"CMD_TIP_OFFSET",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(TIP_OFFSET_CONFIG),	CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_INCREMENT:
			g_cmdinfo[i] = { CMD_INCREMENT,		"CMD_INCREMENT",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(INCREMENT_CONFIG),	CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_FTT_MODE:
			g_cmdinfo[i] = { CMD_FTT_MODE,		"CMD_FTT_MODE",		SET_GET_RESET,	SCOPE_SENSOR,	sizeof(ENUM_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_BORESIGHT:
			g_cmdinfo[i] = { CMD_BORESIGHT,		"CMD_BORESIGHT",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(BORESIGHT_CONFIG),	CFG_RUNTIME,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_SENSOR_WHOAMI:
			g_cmdinfo[i] = { CMD_SENSOR_WHOAMI,	"CMD_SENSOR_WHOAMI",GETONLY,		SCOPE_SENSOR,	sizeof(WHOAMI_STRUCT),		CFG_NA,			CMDTYPE_RO, NO_TAG_EQUIV };
			break;
		case CMD_FRAMERATE:
			g_cmdinfo[i] = { CMD_FRAMERATE,		"CMD_FRAMERATE",	SET_GET_RESET,	SCOPE_SEU,		sizeof(FRAMERATE_CONFIG),	CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_UNITS:
			g_cmdinfo[i] = { CMD_UNITS,			"CMD_UNITS",		SET_GET_RESET,	SCOPE_SEU,		sizeof(UNITS_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_SRC_ROTATION:
			g_cmdinfo[i] = { CMD_SRC_ROTATION,	"CMD_SRC_ROTATION",	SET_GET_RESET,	SCOPE_SEU,		sizeof(SRCROT_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SYNC_MODE:
			g_cmdinfo[i] = { CMD_SYNC_MODE,		"CMD_SYNC_MODE",	SET_GET_RESET,	SCOPE_SEU,		sizeof(BINARY_CONFIG),		CFG_PERSIST,	CMDTYPE_BINCFG, (uint32_t)-1 };
			break;
		case CMD_STATION_MAP:
			g_cmdinfo[i] = { CMD_STATION_MAP,	"CMD_STATION_MAP",	GETONLY,		SCOPE_SEU,		sizeof(STATION_MAP),		CFG_NA,			CMDTYPE_RO, NO_TAG_EQUIV };
			break;
		case CMD_STYLUS:
			g_cmdinfo[i] = { CMD_STYLUS,		"CMD_STYLUS",		SET_GET_RESET,	SCOPE_SEU,		sizeof(STYLUS_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SEUID:
			g_cmdinfo[i] = { CMD_SEUID,			"CMD_SEUID",		SET_GET_RESET,	SCOPE_SEU,		sizeof(VIPER_SEUID),		CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_DUAL_OUTPUT:
			g_cmdinfo[i] = { CMD_DUAL_OUTPUT,	"CMD_DUAL_OUTPUT",	SET_GET_RESET,	SCOPE_SEU,		sizeof(BINARY_CONFIG),		CFG_PERSIST,	CMDTYPE_BINCFG, (uint32_t)-1 };
			break;
		case CMD_SERIAL_CONFIG:
			g_cmdinfo[i] = { CMD_SERIAL_CONFIG,	"CMD_SERIAL_CONFIG",SET_GET_RESET,	SCOPE_SEU,		sizeof(SERIAL_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_BLOCK_CFG:
			g_cmdinfo[i] = { CMD_BLOCK_CFG,		"CMD_BLOCK_CFG",	RESETONLY,		SCOPE_SEU,		sizeof(BLOCK_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, NO_TAG_EQUIV };
			break;
		case CMD_FRAME_COUNT:
			g_cmdinfo[i] = { CMD_FRAME_COUNT,	"CMD_FRAME_COUNT",	RESETONLY,		SCOPE_SEU,		0,							CFG_NA,			CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_BIT:
			g_cmdinfo[i] = { CMD_BIT,			"CMD_BIT",			GETONLY,		SCOPE_SEU,		sizeof(BIT_STRUCT),			CFG_NA,			CMDTYPE_EXEC, (uint32_t)-1 };
			break;
		case CMD_SINGLE_PNO:
			g_cmdinfo[i] = { CMD_SINGLE_PNO,	"CMD_SINGLE_PNO",	GETONLY,		SCOPE_SEU,		MAX_VP_PNO_FRAME_SIZE,		CFG_RUNTIME,	CMDTYPE_PNO, NO_TAG_EQUIV };
			break;
		case CMD_CONTINUOUS_PNO:
			g_cmdinfo[i] = { CMD_CONTINUOUS_PNO,"CMD_CONTINUOUS_PNO",SET_RESET,		SCOPE_SEU,		sizeof(CONT_FC_CONFIG),		CFG_RUNTIME,	CMDTYPE_PNO, NO_TAG_EQUIV };
			break;
		case CMD_WHOAMI:
			g_cmdinfo[i] = { CMD_WHOAMI,		"CMD_WHOAMI",		GETONLY,		SCOPE_SEU,		sizeof(WHOAMI_STRUCT),		CFG_NA,			CMDTYPE_RO, NO_TAG_EQUIV };
			break;
		case CMD_INITIALIZE:
			g_cmdinfo[i] = { CMD_INITIALIZE,	"CMD_INITIALIZE",	SETONLY,		SCOPE_SEU,		0,							CFG_NA,			CMDTYPE_EXEC, (uint32_t)-1 };
			break;
		case CMD_PERSIST:
			g_cmdinfo[i] = { CMD_PERSIST,		"CMD_PERSIST",		SETONLY,		SCOPE_SEU,		0,							CFG_NA,			CMDTYPE_EXEC, NO_TAG_EQUIV };
			break;
		case CMD_PREDFILTER_CFG:
			g_cmdinfo[i] = { CMD_PREDFILTER_CFG,"CMD_PREDFILTER_CFG",SET_GET_RESET,	SCOPE_SENSOR,	sizeof(PF_CONFIG),			CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_PREDFILTER_EXT:
			g_cmdinfo[i] = { CMD_PREDFILTER_EXT,"CMD_PREDFILTER_EXT",SET_GET_RESET,	SCOPE_SENSOR,	sizeof(PF_CONFIG_EXT),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SRC_SELECT:
			g_cmdinfo[i] = { CMD_SRC_SELECT,	"CMD_SRC_SELECT",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(SRC_SEL_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SNS_ORIGIN:
			g_cmdinfo[i] = { CMD_SNS_ORIGIN,	"CMD_SNS_ORIGIN",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(ENUM_CONFIG),		CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SNS_VIRTUAL:
			g_cmdinfo[i] = { CMD_SNS_VIRTUAL,	"CMD_SNS_VIRTUAL",	SET_GET_RESET,	SCOPE_SENSOR,	sizeof(VIRTUAL_SNS_CONFIG),	CFG_PERSIST,	CMDTYPE_CFG, (uint32_t)-1 };
			break;
		case CMD_SRC_WHOAMI:
			g_cmdinfo[i] = { CMD_SRC_WHOAMI,	"CMD_SRC_WHOAMI",	GETONLY,		SCOPE_SEU,		sizeof(WHOAMI_STRUCT),		CFG_NA,			CMDTYPE_RO, (uint32_t)-1 };
			break;
		};
	}
}

void CVPcmd::InitActInfo()
{
	// Don't initialize it if it is already filled.
	if (g_actinfo[0].act != CMD_ACTION_MAX)
		return;

	for (int i = 0; i <= CMD_ACTION_MAX; i++)
	{
		switch (i)
		{
			/**				eAction					str	*/
			/**-----------------------------------------*/
		default:
			g_actinfo[i] = { CMD_ACTION_MAX, "ACTION_UNDEFINED" };
			break;
		case CMD_ACTION_SET:
			g_actinfo[i] = { CMD_ACTION_SET, "CMD_ACTION_SET" };
			break;/**< Used with a command and payload to change state. */
		case CMD_ACTION_GET:
			g_actinfo[i] = { CMD_ACTION_GET, "CMD_ACTION_GET" };
			break;              /**< Used with a command to request current state. */
		case CMD_ACTION_RESET:
			g_actinfo[i] = { CMD_ACTION_RESET, "CMD_ACTION_RESET" };
			break;           /**< Used with a command to restore default state. */
		case CMD_ACTION_ACK:
			g_actinfo[i] = { CMD_ACTION_ACK, "CMD_ACTION_ACK" };
			break;             /**< Command received and processed, possible payload included. */
		case CMD_ACTION_NAK:
			g_actinfo[i] = { CMD_ACTION_NAK, "CMD_ACTION_NAK" };
			break;             /**< Command received, error found: always includes NAK_INFO struct in payload. */
		case CMD_ACTION_NAK_WARNING:
			g_actinfo[i] = { CMD_ACTION_NAK_WARNING, "CMD_ACTION_NAK_WARNING" };
			break;             /**< Command received and processed, warning condition: payload includes NAK_INFO struct */
		};
	}
}

void CVPcmd::InitUnitsInfo()
{
	if ((g_oriuinfo[0].eu != ORI_MAX) && (g_posuinfo[0].eu != POS_MAX))
		return;

	for (int i=0; i <= POS_MAX; i++)
	{
		switch (i)
		{
		default:
			g_posuinfo[i] = { POS_MAX, "POS_UNITS_UNDEFINED", "", "" };
			break;
		case POS_INCH: // = 0 /**< Inches. */
			g_posuinfo[i] = { POS_INCH, "POS_INCH", "Inch", "in" };
			break;
		case POS_FOOT: //   /**< Feet. */
			g_posuinfo[i] = { POS_FOOT, "POS_FOOT", "Foot", "ft" };
			break;
		case POS_CM:     /**< Centimeters. */
			g_posuinfo[i] = { POS_CM, "POS_CM", "CM", "cm" };
			break;
		case POS_METER:  /**< Meters. */
			g_posuinfo[i] = { POS_METER, "POS_METER", "M", "m " };
			break;
		}
	}

	for (int i=0; i <= ORI_MAX; i++)
	{
		switch (i)
		{
		default:
			g_oriuinfo[i] = { ORI_MAX, "ORI_UNITS_UNKNOWN", "" };
			break;
		case ORI_EULER_DEGREE:// = 0 /**< Euler angles in degrees. */
			g_oriuinfo[i] = { ORI_EULER_DEGREE, "ORI_EULER_DEGREE", "Deg", "deg" };
			break;
		case ORI_EULER_RADIAN:   /**< Euler angles in radians. */
			g_oriuinfo[i] = { ORI_EULER_RADIAN, "ORI_EULER_RADIAN", "Rad", "rad" };
			break;
		case ORI_QUATERNION:     /**< Quaternions. */
			g_oriuinfo[i] = { ORI_QUATERNION, "ORI_QUATERNION", "Quat", "qtrn" };
			break;

		}
	}
}

void CVPcmd::InitFRateInfo()
{
	if (g_frateinfo[0].efr != FR_MAX) 
		return;

	for (int i = 0; i <= FR_MAX; i++)
	{
		switch (i)
		{
		default:
			g_frateinfo[i] = { FR_MAX, "FRAMERATE_UNDEFINED" };
			break;
		case FR_30: 
			g_frateinfo[i] = { FR_30, "FR_30" };
			break;
		case FR_60: 
			g_frateinfo[i] = { FR_60, "FR_60" };
			break;
		case FR_120: 
			g_frateinfo[i] = { FR_120, "FR_120" };
			break;
		case FR_240: 
			g_frateinfo[i] = { FR_240, "FR_240" };
			break;
		case FR_480: 
			g_frateinfo[i] = { FR_480, "FR_480" };
			break;
		case FR_960: 
			g_frateinfo[i] = { FR_960, "FR_960" };
			break;
		}
	}
}

void CVPcmd::InitFilterLevInfo()
{
	if (g_flevinfo[0].el != FILTER_LVL_MAX)
		return;

	for (int i = 0; i <= FILTER_LVL_MAX; i++)
	{			
		switch (i)
		{
		default:
			g_flevinfo[i] = { FILTER_LVL_MAX, "FILTER_LVL_UNDEFINED" };
			break;
		case FILTER_LVL_NONE:		/**< 0 no filtering */
			g_flevinfo[i] = { FILTER_LVL_NONE, "FILTER_LVL_NONE" };
			break;
		case FILTER_LVL_LIGHT:		/**< 1 light filtering */
			g_flevinfo[i] = { FILTER_LVL_LIGHT, "FILTER_LVL_LIGHT" };
			break;
		case FILTER_LVL_MEDIUM:		/**< 2 medium filtering */
			g_flevinfo[i] = { FILTER_LVL_MEDIUM, "FILTER_LVL_MEDIUM" };
			break;
		case FILTER_LVL_HEAVY:		/**< 3 heavy filtering */
			g_flevinfo[i] = { FILTER_LVL_HEAVY, "FILTER_LVL_HEAVY" };
			break;
		case FILTER_LVL_CUSTOM:		/**< 4 custom filtering */
			g_flevinfo[i] = { FILTER_LVL_CUSTOM , "FILTER_LVL_CUSTOM" };
			break;

		case FILTER_LVL_E_LIGHT:	/**< 5 enhanced light filtering */
			g_flevinfo[i] = { FILTER_LVL_E_LIGHT, "FILTER_LVL_E_LIGHT" };
			break;
		case FILTER_LVL_E_MEDIUM:	/**< 6 enhanced medium filtering */
			g_flevinfo[i] = { FILTER_LVL_E_MEDIUM, "FILTER_LVL_E_MEDIUM" };
			break;
		case FILTER_LVL_E_HEAVY:	/**< 7 enhanced heavy filtering */
			g_flevinfo[i] = { FILTER_LVL_E_HEAVY, "FILTER_LVL_E_HEAVY" };
			break;
		case FILTER_LVL_RESERVED:	/**< 8 reserved */
			g_flevinfo[i] = { FILTER_LVL_RESERVED , "FILTER_LVL_RESERVED" };
			break;
		}
	}
}

void CVPcmd::InitFilterTarInfo()
{
	if (g_ftarinfo[0].et != FILTER_TRGT_MAX)
		return;

	for (int i = 0; i <= FILTER_TRGT_MAX; i++)
	{
		switch (i)
		{
		default:
			g_ftarinfo[i] = { FILTER_TRGT_MAX, "FILTER_TRGT_UNDEFINED" };
			break;
		case FILTER_TRGT_PNO: // = 0 /**< Target position and orientation */
			g_ftarinfo[i] = { FILTER_TRGT_PNO, "FILTER_TRGT_PNO" };
			break;
		case FILTER_TRGT_ORI: //   /**< Target orientation */
			g_ftarinfo[i] = { FILTER_TRGT_ORI, "FILTER_TRGT_ORI" };
			break;
		case FILTER_TRGT_POS:     /**< Target position */
			g_ftarinfo[i] = { FILTER_TRGT_POS, "FILTER_TRGT_POS" };
			break;
		}
	}
}

void CVPcmd::InitFTTInfo()
{
	if (g_fttinfo[0].eftt != FTT_MODE_MAX)
		return;

	for (int i = 0; i <= FTT_MODE_MAX; i++)
	{
		switch (i)
		{
		default:
			g_fttinfo[i] = { FTT_MODE_MAX, "FTT_MODE_UNDEFINED" };
			break;
		case FTT_MODE_NONE: // = 0 /**< Target position and orientation */
			g_fttinfo[i] = { FTT_MODE_NONE, "FTT_MODE_NONE" };
			break;
		case FTT_MODE_1: //   /**< Target orientation */
			g_fttinfo[i] = { FTT_MODE_1, "FTT_MODE_1" };
			break;
		case FTT_MODE_2:     /**< Target position */
			g_fttinfo[i] = { FTT_MODE_2, "FTT_MODE_2" };
			break;
		case FTT_MODE_3:     /**< Target position */
			g_fttinfo[i] = { FTT_MODE_3, "FTT_MODE_3" };
			break;
		}
	}
}

void CVPcmd::InitHemisphereInfo()
{
	if (g_heminfo[0].et != CHemisphereCfg::HEMPRESETVAL_MAX)
		return;

	for (int i = 0; i <= CHemisphereCfg::HEMPRESETVAL_MAX; i++)
	{
		switch (i)
		{
		default:
			g_heminfo[i] = { (uint32_t)CHemisphereCfg::HEMPRESETVAL_MAX, "HEMPRESETVAL_MAX", {0, 0, 0, 0.0f, 0.0f, 0.0f} };
			break;
		case CHemisphereCfg::POS_X: // = 0 /**< Target position and orientation */
			g_heminfo[i] = { CHemisphereCfg::POS_X, "POS_X", {0, 0, 0, 1.0f, 0.0f, 0.0f} };
			break;
		case CHemisphereCfg::POS_Y: // = 0 /**< Target position and orientation */
			g_heminfo[i] = { CHemisphereCfg::POS_Y, "POS_Y",{ 0, 0, 0, 0.0f, 1.0f, 0.0f } };
			break;
		case CHemisphereCfg::POS_Z: // = 0 /**< Target position and orientation */
			g_heminfo[i] = { CHemisphereCfg::POS_Z, "POS_Z",{ 0, 0, 0, 0.0f, 0.0f, 1.0f } };
			break;
		case CHemisphereCfg::NEG_X: //   /**< Target orientation */
			g_heminfo[i] = { CHemisphereCfg::NEG_X, "NEG_X",{ 0, 0, 0, -1.0f, 0.0f, 0.0f } };
			break;
		case CHemisphereCfg::NEG_Y: //   /**< Target orientation */
			g_heminfo[i] = { CHemisphereCfg::NEG_Y, "NEG_Y",{ 0, 0, 0, 0.0f, -1.0f, 0.0f } };
			break;
		case CHemisphereCfg::NEG_Z: //   /**< Target orientation */
			g_heminfo[i] = { CHemisphereCfg::NEG_Z, "NEG_Z",{ 0, 0, 0, 0.0f, 0.0f, -1.0f } };
			break;
		//case CHemisphereCfg::AUTO_HEM: //   /**< Target orientation */
		//	g_heminfo[i] = { CHemisphereCfg::AUTO_HEM, "Auto",{ 1, 1, 0, 0.0f, 0.0f, 0.0f } };
		//	break;
		case CHemisphereCfg::CUSTOM: //   /**< Target orientation */
			g_heminfo[i] = { CHemisphereCfg::CUSTOM, "CUSTOM",{ 0, 0, 0, 0.0f, 0.0f, 0.0f } };
			break;
		//case CHemisphereCfg::TRACK_EN: //   /**< Target orientation */
		//	g_heminfo[i] = { CHemisphereCfg::TRACK_EN, "HEM_TRACKING_ENABLED",{ 1, 0, 0, 0.0f, 0.0f, 0.0f } };
			break;
		}
	}
}

void CVPcmd::InitViperHemisphereInfo()
{
	if (g_vpheminfo[0].ehem != E_VP_HEM_MAX)
		return;

	for (int i = 0; i <= E_VP_HEM_MAX; i++)
	{
		switch (i)
		{
		default:
			g_vpheminfo[i] = { E_VP_HEM_MAX, "E_VP_HEM_MAX" };
			break;
		case E_VP_HEM_POS_X: // = 0 /**< +X */
			g_vpheminfo[i] = { E_VP_HEM_POS_X, "POS_X" };
			break;
		case E_VP_HEM_POS_Y: // = 0 /**< +Y */
			g_vpheminfo[i] = { E_VP_HEM_POS_Y, "POS_Y" };
			break;
		case E_VP_HEM_POS_Z: // = 0 /**< +Z */
			g_vpheminfo[i] = { E_VP_HEM_POS_Z, "POS_Z" };
			break;
		case E_VP_HEM_NEG_X: //   /**< -X */
			g_vpheminfo[i] = { E_VP_HEM_NEG_X, "NEG_X" };
			break;
		case E_VP_HEM_NEG_Y: //   /**< -Y */
			g_vpheminfo[i] = { E_VP_HEM_NEG_Y, "NEG_Y" };
			break;
		case E_VP_HEM_NEG_Z: //   /**< -Z */
			g_vpheminfo[i] = { E_VP_HEM_NEG_Z, "NEG_Z" };
			break;
		case E_VP_HEM_AUTO: //   /**< Auto-Hemisphere */
			g_vpheminfo[i] = { E_VP_HEM_AUTO, "Auto" };
			break;
		}
	}
}

void CVPcmd::InitBaudInfo()
{
	// Don't initialize it if it is already filled.
	if (g_baudinfo[0].eb != E_BR_MAX)
		return;

	for (int i = 0; i <= E_BR_MAX; i++)
	{
		switch (i)
		{
			/**				eBaud					str	*/
			/**-----------------------------------------*/
		default:
			g_baudinfo[i] = { E_BR_MAX, "BAUD_UNDEFINED", "undef", 0 };
			break;
		case E_BR_1200:
			g_baudinfo[i] = { E_BR_1200, "E_BR_1200", "1200", 1200 };/**< Not used by viper. */
			break;
		case E_BR_2400:
			g_baudinfo[i] = { E_BR_2400, "E_BR_2400", "2400", 2400 }; /**< Not used by viper. */
			break;              
		case E_BR_4800:
			g_baudinfo[i] = { E_BR_4800, "E_BR_4800", "4800", 4800 };/**< Not used by viper. */
			break;           
		case E_BR_9600:
			g_baudinfo[i] = { E_BR_9600, "E_BR_9600", "9600", 9600 };/**< Not used by viper. */
			break;             
		case E_BR_19200:
			g_baudinfo[i] = { E_BR_19200, "E_BR_19200", "19200", 19200 };/**< Not used by viper. */
			break;             
		case E_BR_38400:
			g_baudinfo[i] = { E_BR_38400, "E_BR_38400", "38400", 38400 };
			break;
		case E_BR_57600:
			g_baudinfo[i] = { E_BR_57600, "E_BR_57600", "57600", 57600 };
			break;
		case E_BR_115200:
			g_baudinfo[i] = { E_BR_115200, "E_BR_115200", "115200", 115200 };
			break;
		case E_BR_230400:
			g_baudinfo[i] = { E_BR_230400, "E_BR_230400", "230400", 230400 };
			break;
		case E_BR_460800:
			g_baudinfo[i] = { E_BR_460800, "E_BR_460800", "460800", 460800 };
			break;
		case E_BR_921600:
			g_baudinfo[i] = { E_BR_921600, "E_BR_921600", "921600", 921600 };
			break;
		case E_BR_1843200:
			g_baudinfo[i] = { E_BR_1843200, "E_BR_1843200", "1843200", 1843200 };
			break;
		case E_BR_3686400:
			g_baudinfo[i] = { E_BR_3686400, "E_BR_3686400", "3686400", 3686400 };
			break;
		case E_BR_7372800:
			g_baudinfo[i] = { E_BR_7372800, "E_BR_7372800", "7372800", 7372800 };
			break;
		};
	}
}

void CVPcmd::InitParityInfo()
{
	// Don't initialize it if it is already filled.
	if (g_parityinfo[0].ep != E_PARITY_MAX)
		return;

	for (int i = 0; i <= E_PARITY_MAX; i++)
	{
		switch (i)
		{
			/**				eParity					estr	valstr*/
			/**---------------------------------------------------*/
		default:
			g_parityinfo[i] = { E_PARITY_MAX, "E_PARITY_MAX", "undef" };
			break;
		case E_PARITY_NONE:
			g_parityinfo[i] = { E_PARITY_NONE, "E_PARITY_NONE", "none" };
			break;
		case E_PARITY_ODD:
			g_parityinfo[i] = { E_PARITY_ODD, "E_PARITY_ODD", "odd" }; 
			break;
		case E_PARITY_EVEN:
			g_parityinfo[i] = { E_PARITY_EVEN, "E_PARITY_EVEN", "even" };
			break;
		};
	}
}

void CVPcmd::InitBITcodeInfo()
{
	// Don't initialize it if it is already filled.
	if (g_bitcodeinfo[0].ep != E_BITERR_MAX)
		return;

	for (int i = 0; i <= E_BITERR_MAX; i++)
	{
		switch (i)
		{
			/**				eBITcode					estr	valstr*/
			/**---------------------------------------------------*/
		default:
			g_bitcodeinfo[i] = { E_BITERR_MAX,         "E_BITERR_MAX",         "Undefined error"              };
			break;
		case E_BITERR_NONE:   //!< No Error.  Self-test and startup successful.
			g_bitcodeinfo[i] = { E_BITERR_NONE,        "E_BITERR_NONE",        "No Error"                     };
			break;
		case E_BITERR_SENSOR:   //!< Sensor error
			g_bitcodeinfo[i] = { E_BITERR_SENSOR,      "E_BITERR_SENSOR",      "SENSOR"                 };
			break;
		case E_BITERR_SENSOR_PORT:   //!< Sensor PORT error
			g_bitcodeinfo[i] = { E_BITERR_SENSOR_PORT, "E_BITERR_SENSOR_PORT", "SENSOR PORT"            };
			break;
		case E_BITERR_SOURCE:   //!< Source and/or Source PORT error
			g_bitcodeinfo[i] = { E_BITERR_SOURCE,      "E_BITERR_SOURCE",      "SOURCE"                 };
			break;
		case E_BITERR_SEU:   //!< SEU error
			g_bitcodeinfo[i] = { E_BITERR_SEU,         "E_BITERR_SEU",         "SEU"                    };
			break;
		case E_BITERR_DUPFREQ:   //!< Duplicate Source Frequencies detected.
			g_bitcodeinfo[i] = { E_BITERR_DUPFREQ,     "E_BITERR_DUPFREQ",     "DUP SOURCE FREQ" };
			break;
		case E_BITERR_RESERVED1:   //!< Reserved.
			g_bitcodeinfo[i] = { E_BITERR_RESERVED1,   "E_BITERR_RESERVED1",   "Reserved 1"              };
			break;
		case E_BITERR_RESERVED2:   //!< Reserved.
			g_bitcodeinfo[i] = { E_BITERR_RESERVED2,   "E_BITERR_RESERVED2",   "Reserved 2"              };
			break;
		}
	}
}eBITcode;

void CVPcmd::InitStylusInfo()
{
	// Don't initialize it if it is already filled.
	if (g_stylusinfo[0].e != STYLUS_MODE_MAX)
		return;

	for (int i = 0; i <= STYLUS_MODE_MAX; i++)
	{
		switch (i)
		{
			/**				eStylusMode				str	*/
			/**-----------------------------------------*/
		default:
			g_stylusinfo[i] = { STYLUS_MODE_MAX, "STYLUS_MODE_MAX" };
			break;
		case STYLUS_MODE_MARK:
			g_stylusinfo[i] = { STYLUS_MODE_MARK, "STYLUS_MODE_MARK" };
			break;
		case STYLUS_MODE_POINT:
			g_stylusinfo[i] = { STYLUS_MODE_POINT, "STYLUS_MODE_POINT" };
			break;
		case STYLUS_MODE_LINE:
			g_stylusinfo[i] = { STYLUS_MODE_LINE, "STYLUS_MODE_LINE" };
			break;
		case STYLUS_MODE_TOGGLE:
			g_stylusinfo[i] = { STYLUS_MODE_TOGGLE, "STYLUS_MODE_TOGGLE" };
			break;
		};
	}
}

void CVPcmd::InitSnsOrigInfo()
{
	// Don't initialize it if it is already filled.
	if (g_snsoriginfo[0].e != SNS_ORIG_MAX)
		return;

	for (int i = 0; i <= SNS_ORIG_MAX; i++)
	{
		switch (i)
		{
			/**				eStylusMode				str	*/
			/**-----------------------------------------*/
		default:
			g_snsoriginfo[i] = { SNS_ORIG_MAX, "SNS_ORIG_MAX" };
			break;
		case SNS_ORIG_SRC1:
			g_snsoriginfo[i] = { SNS_ORIG_SRC1, "SNS_ORIG_SRC1" };
			break;
		case SNS_ORIG_SRC2:
			g_snsoriginfo[i] = { SNS_ORIG_SRC2, "SNS_ORIG_SRC2" };
			break;
		case SNS_ORIG_SRC3:
			g_snsoriginfo[i] = { SNS_ORIG_SRC3, "SNS_ORIG_SRC3" };
			break;
		case SNS_ORIG_SRC4:
			g_snsoriginfo[i] = { SNS_ORIG_SRC4, "SNS_ORIG_SRC4" };
			break;
		case SNS_ORIG_COMMON:
			g_snsoriginfo[i] = { SNS_ORIG_COMMON, "SNS_ORIG_COMMON" };
			break;
		};
	}
}

/*inline*/ bool CVPcmd::ValidateLeader(uint8_t * pbuf, uint32_t cmdPre/* = VIPER_CMD_PREAMBLE*/, uint32_t pnoPre/* = VIPER_PNO_PREAMBLE*/)
{
	bool bRet = false;
	VP_FRAME_HDR* pF = (VP_FRAME_HDR*)pbuf;

	if (!pF)
	{
	}
	else if (pF->preamble != cmdPre && pF->preamble != pnoPre)
	{
	}
	else
	{
		if (pF->preamble == pnoPre)
		{
			if ((pF->size < VP_PNO_FRAME_SIZE_FIELD_MIN) || (pF->size > VP_PNO_FRAME_SIZE_FIELD_MAX))
			{
			}
			else
				bRet = true;

		}
		else if (pF->preamble == cmdPre)
		{
			if ((pF->size < VP_CMD_FRAME_SIZE_FIELD_MIN) || (pF->size > VP_CMD_FRAME_SIZE_FIELD_MAX))
			{
			}
			else
				bRet = true;

		}
		else
		{
		}
	}

	return bRet;
}

/*inline*/ size_t CVPcmd::paysize(eViperCmds cmd)
{
	if (g_cmdinfo[0].cmd == CMD_MAX)
		InitCmdInfo();

	if (cmd < CMD_MAX)
		return g_cmdinfo[(int)cmd].payload_size;
	else
		return 0;
}

/*inline*/ size_t CVPcmd::framesize(eViperCmds cmd)
{
	return sizeof(SEUCMD_HDR) + paysize(cmd) + sizeof(uint32_t);
}

/*inline*/ const char * CVPcmd::cmdstr(uint32_t cmd)
{
	if (g_cmdinfo[0].cmd == CMD_MAX)
		InitCmdInfo();

	if (cmd < CMD_MAX)
		return g_cmdinfo[cmd].szcmd;
	else
		return "undefined_cmd";
}

/*inline*/ const char * CVPcmd::actstr(uint32_t act)
{
	if (g_actinfo[0].act == CMD_ACTION_MAX)
		InitActInfo();

	if (act < CMD_ACTION_MAX)
		return g_actinfo[act].szact;
	else
		return "undefined_action";
}

/*inline*/ const char * CVPcmd::posunitsstr(uint32_t posu, bool bpub/*=false*/, bool bbrief/*=false*/)
{
	if (g_posuinfo[0].eu == POS_MAX)
		InitUnitsInfo();

	if (bpub)
		return g_posuinfo[CLAMP(posu, 0, POS_MAX)].szu_pub;
	else if (bbrief)
		return g_posuinfo[CLAMP(posu, 0, POS_MAX)].szu_brief;

	return g_posuinfo[CLAMP(posu, 0, POS_MAX)].szu;
	//return bpub ? g_posuinfo[CLAMP(posu, 0, POS_MAX)].szu_pub : g_posuinfo[CLAMP(posu, 0, POS_MAX)].szu;
}

/*inline*/ const char * CVPcmd::oriunitsstr(uint32_t oriu, bool bpub/*=false*/, bool bbrief /*= false*/)
{
	if (g_oriuinfo[0].eu == ORI_MAX)
		InitUnitsInfo();

	if (bpub)
		return g_oriuinfo[CLAMP(oriu, 0, ORI_MAX)].szu_pub;
	else if (bbrief)
		return g_oriuinfo[CLAMP(oriu, 0, ORI_MAX)].szu_brief;

	return g_oriuinfo[CLAMP(oriu, 0, ORI_MAX)].szu;

	//return bpub ? g_oriuinfo[CLAMP(oriu, 0, ORI_MAX)].szu_pub : g_oriuinfo[CLAMP(oriu, 0, ORI_MAX)].szu;
}

/*inline*/ const char * CVPcmd::frameratestr(uint32_t frate)
{
	if (g_frateinfo[0].efr == FR_MAX)
		InitFRateInfo();

	return g_frateinfo[CLAMP(frate, 0, FR_MAX)].szfr;
}

/*inline*/ const char * CVPcmd::filterlevstr(uint32_t flev)
{
	if (g_flevinfo[0].el == FILTER_LVL_MAX)
		InitFilterLevInfo();

	return g_flevinfo[CLAMP(flev, 0, FILTER_LVL_MAX)].szl;
}

/*inline*/ const char * CVPcmd::filtertarstr(uint32_t ftar)
{
	if (g_ftarinfo[0].et == FILTER_TRGT_MAX)
		InitFilterTarInfo();

	return g_ftarinfo[CLAMP(ftar, 0, FILTER_TRGT_MAX)].szt;
}
/*inline*/ const char * CVPcmd::fttstr(uint32_t ftt)
{
	if (g_fttinfo[0].eftt == FTT_MODE_MAX)
		InitFTTInfo();

	return g_fttinfo[CLAMP(ftt, 0, FTT_MODE_MAX)].szftt;
}

/*inline*/ const char * CVPcmd::hemispherestr(uint32_t hemval)
{
	if (g_heminfo[0].et == CHemisphereCfg::HEMPRESETVAL_MAX)
		InitHemisphereInfo();

	return g_heminfo[CLAMP(hemval, 0, CHemisphereCfg::HEMPRESETVAL_MAX)].szt;

}

/*inline*/ const char * CVPcmd::vphemispherestr(uint32_t hemval)
{
	if (g_vpheminfo[0].ehem == E_VP_HEM_MAX)
		InitViperHemisphereInfo();

	return g_vpheminfo[CLAMP(hemval, 0, E_VP_HEM_MAX)].szhem;
}

/*inline*/ const char * CVPcmd::baudstr(uint32_t b)
{
	if (g_baudinfo[0].eb == E_BR_MAX)
		InitBaudInfo();

	if (b < E_BR_MAX)
		return g_baudinfo[b].szeb;
	else
		return "undefined_baud";
}


/*inline*/ const char * CVPcmd::baudvalstr(uint32_t b)
{
	if (g_baudinfo[0].eb == E_BR_MAX)
		InitBaudInfo();

	if (b < E_BR_MAX)
		return g_baudinfo[b].szval;
	else
		return "undefined";
}

/*inline*/ const char * CVPcmd::paritystr(uint32_t p)
{
	if (g_parityinfo[0].ep == E_PARITY_MAX)
		InitParityInfo();

	if (p < E_PARITY_MAX)
		return g_parityinfo[p].szep;
	else
		return "undefined_parity";
}


/*inline*/ const char * CVPcmd::parityvalstr(uint32_t p)
{
	if (g_parityinfo[0].ep == E_PARITY_MAX)
		InitParityInfo();

	if (p < E_PARITY_MAX)
		return g_parityinfo[p].szval;
	else
		return "undefined";
}

/*inline*/ const char * CVPcmd::bitcodestr(uint32_t p)
{
	if (g_bitcodeinfo[0].ep == E_BITERR_MAX)
		InitBITcodeInfo();

	if (p < E_BITERR_MAX)
		return g_bitcodeinfo[p].szep;
	else
		return "undefined_bit_code";
}


/*inline*/ const char * CVPcmd::bitcodevalstr(uint32_t p)
{
	if (g_bitcodeinfo[0].ep == E_BITERR_MAX)
		InitBITcodeInfo();

	if (p < E_BITERR_MAX)
		return g_bitcodeinfo[p].szval;
	else
		return "undefined";
}

/*inline*/ const char * CVPcmd::stylusmodestr(uint32_t p)
{
	if (g_stylusinfo[0].e == STYLUS_MODE_MAX)
		InitStylusInfo();

	if (p < STYLUS_MODE_MAX)
		return g_stylusinfo[p].sz;
	else
		return "undefined_stylus_mode";
}

/*inline*/ const char * CVPcmd::snsorigstr(uint32_t p)
{
	if (g_snsoriginfo[0].e == SNS_ORIG_MAX)
		InitSnsOrigInfo();

	if (p < SNS_ORIG_MAX)
		return g_snsoriginfo[p].sz;
	else
		return "undefined_sensor_origin";
}

/*inline*/ const HEMISPHERE_CONFIG *CVPcmd::hemispherecfg(uint32_t hemval)
{
	if (g_heminfo[0].et == CHemisphereCfg::HEMPRESETVAL_MAX)
		InitHemisphereInfo();

	return &(g_heminfo[CLAMP(hemval, 0, CHemisphereCfg::HEMPRESETVAL_MAX)].cfg);
}

/*inline*/ const int CVPcmd::baudbps(uint32_t b)
{
	if (g_baudinfo[0].eb == E_BR_MAX)
		InitBaudInfo();

	return g_baudinfo[CLAMP(b, 0, E_BR_MAX)].bps;

}

/*inline*/ const eBaud CVPcmd::bpsbaud(int32_t bps)
{
	if (g_baudinfo[0].eb == E_BR_MAX)
		InitBaudInfo();

	for (int i = 0; i < E_BR_MAX; i++)
	{
		if (g_baudinfo[i].bps == bps)
			return g_baudinfo[i].eb;
	}
	return E_BR_MAX;
}

/*inline*/ bool CVPcmd::ValidateAct(eViperCmds cmd, eCmdActions act)
{
	InitActInfo();

	if (g_actinfo[0].act == CMD_ACTION_MAX)
		InitActInfo();
	if (cmd < CMD_MAX)
		return (g_cmdinfo[cmd].actionmap & (1 << act)) != 0;
	else
		return false;
}

/*inline*/ eViperCmds CVPcmd::tag2vpcmd(uint32_t cmd)
{
	InitCmdInfo();

	//if (cmd < TAG_TOTAL_COMMANDS)
	//	return g_cmdinfo_tag[cmd].vpcmd_equiv;
	//else
		return CMD_MAX;
}

/*inline*/ uint32_t CVPcmd::vp2tagcmd(eViperCmds cmd)
{
	InitCmdInfo();

	//if (cmd < CMD_MAX)
	//	return g_cmdinfo[cmd].tag_equiv;
	//else
		return NO_TAG_EQUIV;
}

/*inline*/ void CVPcmd::vp2tag()
{
	seucmd.cmd = vp2tagcmd((eViperCmds)seucmd.cmd);
	//preamble = TAG_CMD_PREAMBLE;
}
#define VIPER_PID 0xBF01

/*inline*/ uint32_t CVPcmd::TrkCmdPre(uint32_t pid)
{
	switch (pid)
	{
	//case TAG_PID:
	//	return TAG_CMD_PREAMBLE;
	//	break;
	case VIPER_PID:
		return VIPER_CMD_PREAMBLE;
		break;
	default:
		break;

	}
	return 0;
}
uint32_t CVPcmd::TrkPnoPre(uint32_t pid)
{
	switch (pid)
	{
	//case G4PLUS_PID:
	//	return G4PLUS_PNO_PREAMBLE;
	//	break;
	case VIPER_PID:
		return VIPER_PNO_PREAMBLE;
		break;
	default:
		break;

	}
	return 0;
}

float CVPSnsFrameA::OriFractFactor[ORI_MAX] = {
	FFACTOR_EULER_DEGREE
	, FFACTOR_EULER_RAD
	, FFACTOR_QTRN
};

/*inline*/ bool CBoresightCfg::Enabled()
{
	// Viper boresight enabled if rotation angles != 0, or unit Qtrn
	//return !(COriVec(params, m_eOriUnits) == COriVec(m_eOriUnits));
	std::vector<float> nullbore(4, 0.0f);
	std::vector<float> thisbore(4, 0.0f);
	thisbore = std::vector<float>{ {params[0], params[1], params[2], params[3]} };
	if (m_eOriUnits == ORI_QUATERNION)
	{
		nullbore.at(0) = 1.0f;
	}

	return !(thisbore == nullbore);
}

STATION_MAP CStationMap::Default = { 0x0000000f };
UNITS_CONFIG CUnitsCfg::Default = { POS_INCH, ORI_EULER_DEGREE };
SRCROT_CONFIG CSrcRotCfg::Default[ORI_MAX] = {
	 0, 0.0f, 0.0f, 0.0f, 0.0f
	,0, 0.0f, 0.0f, 0.0f, 0.0f
	,0, 1.0f, 0.0f, 0.0f, 0.0f };

SRC_CONFIG CSrcCfg::Default[ORI_MAX] = {
	 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0
	,0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0
	,0, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0 };

FRAMERATE_CONFIG CFrameRateCfg::Default = { FR_240 };


FILTER_CONFIG CFilterCfg::NoneDef =    { FILTER_LVL_NONE,     0.0f,  1.0f,  0.0f, 0.0f  };
FILTER_CONFIG CFilterCfg::Light =   { FILTER_LVL_LIGHT,    0.2f,  0.2f,  0.8f, 0.95f };
FILTER_CONFIG CFilterCfg::Medium =  { FILTER_LVL_MEDIUM,   0.05f, 0.05f, 0.8f, 0.95f };
FILTER_CONFIG CFilterCfg::Heavy =   { FILTER_LVL_HEAVY,    0.02f, 0.02f, 0.8f, 0.95f };
FILTER_CONFIG CFilterCfg::Default = { FILTER_LVL_MEDIUM,   0.05f, 0.05f, 0.8f, 0.95f };
FILTER_CONFIG CFilterCfg::ELight  = { FILTER_LVL_E_LIGHT,  0.0f,  0.0f,  0.0f, 0.0f  };
FILTER_CONFIG CFilterCfg::EMedium = { FILTER_LVL_E_MEDIUM, 0.0f,  0.0f,  0.0f, 0.0f  };
FILTER_CONFIG CFilterCfg::EHeavy =  { FILTER_LVL_E_HEAVY,  0.0f,  0.0f,  0.0f, 0.0f  };
FILTER_CONFIG CFilterCfg::EDefault = { FILTER_LVL_E_MEDIUM, 0.0f,  0.0f,  0.0f, 0.0f };

FILTER_CONFIG * CFilterCfg::presets[FILTER_LVL_MAX] = {
	&CFilterCfg::NoneDef
	, &CFilterCfg::Light
	, &CFilterCfg::Medium
	, &CFilterCfg::Heavy
	, &CFilterCfg::Default
	, &CFilterCfg::ELight
	, &CFilterCfg::EMedium
	, &CFilterCfg::EHeavy
	, &CFilterCfg::EDefault
};

TIP_OFFSET_CONFIG CTipoffCfg::Default = { 0.0f, 0.0f, 0.0f };
INCREMENT_CONFIG CIncrementCfg::Default = { 0, 0.0f, 0.0f };
BORESIGHT_CONFIG CBoresightCfg::Default[ORI_MAX] = {
	 0.0f, 0.0f, 0.0f, 0.0f
	,0.0f, 0.0f, 0.0f, 0.0f
	,1.0f, 0.0f, 0.0f, 0.0f };

HEMISPHERE_CONFIG CHemisphereCfg::Default = { 0, 0, 0, 1.0f, 0.0f, 0.0f };
SEUID_CONFIG CSeuIDCfg::Default = { 0 };
BINARY_CONFIG CSyncModeCfg::Default = { 0 };
BINARY_CONFIG CDualOutputCfg::Default = { 0 };
STYLUS_CONFIG CStylusCfg::Default = { STYLUS_MODE_MARK };
SERIAL_CONFIG CSerialCfg::Default = { E_BR_115200 };
ENUM_CONFIG CEnumCfg::Default = { 0 };
FTTMODE_CONFIG CFTTCfg::Default = { FTT_MODE_NONE };
SNS_ORIG_CONFIG CSnsOrigCfg::Default = { SNS_ORIG_SRC1 };

NOISEPROFILE CPFNoise::Default = {
	0.0f ,0.0f ,0.0f ,0.0f		//Rq
	,0.0f ,0.0f ,0.0f ,0.0f
	,0.0f ,0.0f ,0.0f ,0.0f
	,0.0f ,0.0f ,0.0f ,0.0f

	,0.0f ,0.0f ,0.0f 		//Qq
	,0.0f ,0.0f ,0.0f
	,0.0f ,0.0f ,0.0f

	,0.0f ,0.0f ,0.0f 		//Rr
	,0.0f ,0.0f ,0.0f
	,0.0f ,0.0f ,0.0f

	,0.0f ,0.0f ,0.0f 		//Qr
	,0.0f ,0.0f ,0.0f
	,0.0f ,0.0f ,0.0f

};
PF_CONFIG_EXT CPredFiltCfgExt::Default = { 0, 0, (20.0f/1000.0f) , 0.0f, CPFNoise::Default };

SRC_SEL_CONFIG CSrcSelectCfg::Default = { 0x01 };

VIRTUAL_SNS_CONFIG CSnsVirtCfg::Default = { 0, 0, 0, 0 };

SENSOR_CONFIG CSensBlockCfg::Default = {
	CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default
};

SENSOR_CONFIG CBlockCfgSens::Default = {
	CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default
};

BLOCK_CONFIG CBlockCfg::Default = {
	CFrameRateCfg::Default
	,CUnitsCfg::Default
	,CSrcCfg::Default[0]
	,CSrcCfg::Default[0]
	,CSrcCfg::Default[0]
	,CSrcCfg::Default[0]
	,CSyncModeCfg::Default
	,CStationMap::Default
	,CStylusCfg::Default
	,CSeuIDCfg::Default.seuid
	,CDualOutputCfg::Default
	,CSerialCfg::Default.sercfg
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default
	,CHemisphereCfg::Default, CFilterCfg::Default, CFilterCfg::Default, CTipoffCfg::Default, CIncrementCfg::Default, CSrcSelectCfg::Default, CSnsOrigCfg::Default, CPredFiltCfgExt::Default

};