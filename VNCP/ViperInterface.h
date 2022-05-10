/* VSS $Header: /Viper/VPCmdIF/VPCmdIF/Inc/ViperInterface.h 49    3/18/20 3:07p Suzanne $
 */
/**
 *  @file ViperInterface.h
 *
 *  Contains all native structures and enums for managing the Viper SEU directly.
 */
#ifndef VIPERINTERFACE_H_INCLUDED
#define VIPERINTERFACE_H_INCLUDED

#include <stdint.h>
#define VIPERINTERFACE_REV 200519a

//! The maximum number of sensors a single SEU can have.
#define SENSORS_PER_SEU 16

//! The maximum number of sources a single SEU can have.
#define SOURCES_PER_SEU 4

//! The maximum number of sensor processors a single SEU can have.
#define SNSPROC_PER_SEU 4

//! The maximum frame update rate for the tracker.
//! This is also the default frame rate.
#define VIPER_MAX_FRAMES_PER_SEC 240

//! USB VID/PID
#define POLHEMUS_USB_VID 0xf44
#define VIPER_USB_PID 0xBF01

//! @enum eCmdActions
//! Every command frame has an action associated with it. Commands sent to a SEU may use a
//! ACTION_SET, ACTION_GET, or ACTION_RESET. Commands received will contain a ACTION_ACK or
//! a ACTION_NAK. 'ACK' stands for acknowledgement and 'NACK' stands for negative acknowledgement.
typedef enum eCmdActions
{
	CMD_ACTION_SET = 0 /**< Used with a command and payload to change state. */
	,
	CMD_ACTION_GET /**< Used with a command to request current state. */
	,
	CMD_ACTION_RESET /**< Used with a command to restore default state. */
	,
	CMD_ACTION_ACK /**< Command received and processed, possible payload included. */
	,
	CMD_ACTION_NAK /**< Command received, error found: always includes NAK_INFO struct in payload. */
	,
	CMD_ACTION_NAK_WARNING /**< Command received and processed, warning condition: payload includes NAK_INFO struct */

	,
	CMD_ACTION_MAX
} eCmdActions;

//! \anchor anchoreViperCmds
//! @enum eViperCmds
//! Every command frame has a command associated with it.
typedef enum eViperCmds
{				 /** cmd  actions		scope	payload struct		persist		cmd type	arg1/scope?		arg2/qualifier?	*/
				 /**-----------------------------------------------------------------------------------------------------*/
				 // sensor cfg - persistable
  CMD_HEMISPHERE /**  0   set/get/reset,	sensor	HEMISPHERE_CONFIG		p		cfg			sensornum						*/
  ,
  CMD_FILTER /**  1   set/get/reset,	sensor	FILTER_CONFIG			p		cfg			sensornum		eFilterTargets	*/
  ,
  CMD_TIP_OFFSET /**  2   set/get/reset,	sensor	TIP_OFFSET_CONFIG		 		cfg			sensornum						*/
  ,
  CMD_INCREMENT /**  3   set/get/reset,	sensor	INCREMENT_CONFIG		p		cfg			sensornum						*/
				// sensor cfg - runtime
  ,
  CMD_BORESIGHT /**  4   set/get/reset,	sensor	BORESIGHT_CONFIG				cfg			sensornum		bool reset Origin*/
				// sensor cmds - read-only
  ,
  CMD_SENSOR_WHOAMI /**  5      /get/     ,	sensor	WHOAMI_STRUCT					read-only									*/
					// seu cfg - persistable
  ,
  CMD_FRAMERATE /**  6   set/get/reset,	SEU		FRAMERATE_CONFIG(ENUM)	p		cfg											*/
  ,
  CMD_UNITS /**  7   set/get/reset,	SEU		UNITS_CONFIG			p		cfg											*/
  ,
  CMD_SRC_ROTATION /**  8   set/get/reset,	SEU		SRCROT_CONFIG			p		cfg			sourcenum						*/
  ,
  CMD_SYNC_MODE /**  9   set/get/reset,	SEU		BINARY_CONFIG			p		cfg-binary									*/
  ,
  CMD_STATION_MAP /**  10     /get/     ,	SEU		STATION_MAP				p		read-only											*/
  ,
  CMD_STYLUS /**  11  set/get/reset,	SEU		STYLUS_CONFIG(ENUM)		p		cfg											*/
  ,
  CMD_SEUID /**  12  set/get/reset,	SEU		VIPER_SEUID				p		cfg											*/
  ,
  CMD_DUAL_OUTPUT /**  13  set/get/reset,	SEU		BINARY_CONFIG			p		cfg-binary									*/
  ,
  CMD_SERIAL_CONFIG /**  14  set/get/reset,	SEU		SERIAL_CONFIG			p		cfg											*/
  ,
  CMD_BLOCK_CFG /**  15         /reset,	SEU		none					p		cfg											*/
				// seu cfg - runtime
  ,
  CMD_FRAME_COUNT /*  not implemented 16     /   /reset,	SEU		none							executive									*/
  ,
  CMD_BIT /**  17     /get/     ,	SEU		BIT_STRUCT						executive									*/
		  // seu cmds - pno
  ,
  CMD_SINGLE_PNO /**  18  get/   /     ,	SEU		SEUPNO+SENFRAMEDATA				pno							ePnoMode		*/
  ,
  CMD_CONTINUOUS_PNO /**  19  set/   /reset,	SEU		CONT_FC_CONFIG					pno							ePnoMode		*/
					 // seu cmds - read-only
  ,
  CMD_WHOAMI /**  20     /get/     ,	SEU		WHOAMI_STRUCT					read-only									*/
			 // executive cmds
  ,
  CMD_INITIALIZE /**  21  set/   /     ,	SEU		none							executive									*/
  ,
  CMD_PERSIST /**  22  set/   /     ,	SEU		none							executive									*/
  ,
  CMD_ENABLE_MAP /**  23  set/   /reset,	SEU		none							executive									*/

  ,
  CMD_FTT_MODE /**  24  set/get/reset,	sensor	FTTMODE_CONFIG(ENUM)	p		cfg			sensornum						*/
  ,
  CMD_MAP_STATUS /**  25     /get/     ,	SEU																					*/
  ,
  CMD_SENSOR_BLOCKCFG /*  not implemented 26         /reset,	sensor	none					p		cfg			sensornum						*/
  ,
  CMD_SOURCE_CFG /**  27  set/get/reset,	SEU		SRC_CONFIG				p		cfg			sourcenum						*/
  ,
  CMD_PREDFILTER_CFG /**  28  set/get/reset,	sensor	PF_CONFIG				p		cfg			sensornum						*/
  ,
  CMD_PREDFILTER_EXT /**  29  set/get/reset,	sensor	PF_CONFIG_EXT			p		cfg			sensornum						*/
  ,
  CMD_SRC_SELECT /**  30  set/get/reset,	sensor	SRC_SEL_CONFIG			p		cfg			sensornum						*/
  ,
  CMD_SNS_ORIGIN /**  31  set/get/reset,	sensor	SNS_ORIG_CONFIG(ENUM)	p		cfg			sensornum						*/
  ,
  CMD_SNS_VIRTUAL /**  32  set/get/reset,	sensor	VIRTUAL_SNS_CONFIG		p		cfg			sensornum						*/
  ,
  CMD_SRC_WHOAMI /**  33     /get/     ,	source	WHOAMI_STRUCT    		p		read-only	sourcenum						*/
  ,
  CMD_MAX
} eViperCmds;

//! @enum eViperOriUnits
//! \anchor anchoreViperOriUnits
//! Used to identify orientation unit type. Both command frames and PNO frames share the same
//! units, whether they be the defaults or a set value at some later time.
typedef enum eViperOriUnits
{
	ORI_EULER_DEGREE = 0 /**< Euler angles in degrees. */
	,
	ORI_EULER_RADIAN /**< Euler angles in radians. */
	,
	ORI_QUATERNION /**< Quaternions. */

	,
	ORI_MAX
} eViperOriUnits;

//! @enum eViperPosUnits
//! \anchor anchoreViperPosUnits
//! Used to identify position unit type. Both command frames and PNO frames share the same
//! units, whether they be the defaults or a set value at some later time.
typedef enum eViperPosUnits
{
	POS_INCH = 0 /**< Inches. */
	,
	POS_FOOT /**< Feet. */
	,
	POS_CM /**< Centimeters. */
	,
	POS_METER /**< Meters. */

	,
	POS_MAX
} eViperPosUnits;

//! @enum eViperFrameRate
//! \anchor anchoreViperFrameRate
//! Valid Frames per Second (FPS) values for CMD_FRAMERATE in @ref FRAMERATE_CONFIG
typedef enum eViperFrameRate
{
	FR_30 /**< 30 FPS  */
	,
	FR_60 /**< 60 FPS  */
	,
	FR_120 /**< 120 FPS */
	,
	FR_240 /**< 240 FPS */
	,
	FR_480 /**< 480 FPS */
	,
	FR_960 /**< 960 FPS */

	,
	FR_MAX
} eViperFrameRate;

//! @enum eViperFilterTargets
//! \anchor anchoreViperFilterTargets
//! Used to select position or orientation for filtering. The CMD_FILTER command uses this enumeration to determine
//! whether to apply settings to (or get setting for) orientation, position or both.
typedef enum eViperFilterTargets
{
	FILTER_TRGT_PNO = 0 /**< Target position and orientation. Can only be used with CMD_ACTION_SET and CMD_ACTION_RESET. */
	,
	FILTER_TRGT_ORI /**< Target orientation. */
	,
	FILTER_TRGT_POS /**< Target position. */

	,
	FILTER_TRGT_MAX
} eViperFilterTargets;

//! @enum eViperFilterLevel
//! \anchor anchoreViperFilterLevel
//! Used to select the filter level. The default filter preset is FILTER_LVL_MEDIUM. The higher the filter level, the smoother
//! the PNO output. The lower the filter level, the low the latency.
/*! Another enum, with inline docs */
typedef enum eViperFilterLevel
{
	FILTER_LVL_NONE = 0 /**< 0000 @brief no filtering     */
	,
	FILTER_LVL_LIGHT /**< 0001 @brief traditional light filtering  */
	,
	FILTER_LVL_MEDIUM /**< 0002 @brief traditional medium filtering */
	,
	FILTER_LVL_HEAVY /**< 0003 @brief traditional heavy filtering  */
	,
	FILTER_LVL_CUSTOM /**< 0004 @brief traditional custom filtering */
	,
	FILTER_LVL_E_LIGHT /**< 0005 @brief enhanced light filtering  */
	,
	FILTER_LVL_E_MEDIUM /**< 0006 @brief enhanced medium filtering */
	,
	FILTER_LVL_E_HEAVY /**< 0007 @brief enhanced heavy filtering  */
	,
	FILTER_LVL_RESERVED /**< 0008 @brief reserved */

	,
	FILTER_LVL_MAX
} eViperFilterLevel;

//! @enum eStylusMode
//! \anchor anchoreStylusMode
//! Valid values for CMD_STYLUSMODE in @ref STYLUS_CONFIG
typedef enum eStylusMode
{
	STYLUS_MODE_MARK,
	STYLUS_MODE_POINT,
	STYLUS_MODE_LINE,
	STYLUS_MODE_TOGGLE

	,
	STYLUS_MODE_MAX
} eStylusMode;

//! @enum eFTTMode
//! \anchor anchoreFTTMode
//! FTT mode values used for @ref CMD_FTT_MODE
typedef enum eFTTMode
{
	FTT_MODE_NONE = 0,
	FTT_MODE_1 //<! aka Stationary Source Mode
	,
	FTT_MODE_2 //<! aka Moving Source Mode
	,
	FTT_MODE_3

	,
	FTT_MODE_MAX
} eFTTMode;

//! @enum eSensorOrigin
//! \anchor anchoreSensorOrigin
//! Sensor Origin values used for @ref CMD_SNS_ORIGIN
typedef enum eSensorOrigin
{
	SNS_ORIG_SRC1 = 0 /**< Default.  Sensor Origin is local source */
	,
	SNS_ORIG_SRC2 /**< Sensor Origin is Source 2 */
	,
	SNS_ORIG_SRC3 /**< Sensor Origin is Source 3 */
	,
	SNS_ORIG_SRC4 /**< Sensor Origin is Source 4 */
	,
	SNS_ORIG_COMMON /**< Sensor origin is common to the device, usually implied by source configuration */

	,
	SNS_ORIG_MAX
} eSensorOrigin;

//! @enum ePnoMode
//! \anchor anchorePnoMode
//! PNO output mode values used for @ref CMD_SINGLE_PNO and @ref CMD_CONTINUOUS_PNO
typedef enum ePnoMode
{
	E_PNOMODE_STD = 0 //!< standard position and orientation using @ref PNODATA struct per sensor
	,
	E_PNOMODE_A //!< position and orientation plus acceleration using @ref PNO_A struct per sensor

	,
	E_PNOMODE_MAX
} ePnoMode;

//! @enum eViperHemisphere
//! \anchor anchoreViperHemisphere
//! Hemisphere values used for @ref CMD_SOURCE_CFG
typedef enum eViperHemisphere
{
	E_VP_HEM_POS_X = 0 /**<  +X Hemisphere   */
	,
	E_VP_HEM_POS_Y /**<  +Y Hemisphere   */
	,
	E_VP_HEM_POS_Z /**<  +Z Hemisphere   */
	,
	E_VP_HEM_NEG_X /**<  -X Hemisphere   */
	,
	E_VP_HEM_NEG_Y /**<  -Y Hemisphere   */
	,
	E_VP_HEM_NEG_Z /**<  -Z Hemisphere   */
	,
	E_VP_HEM_AUTO /**<  Auto-Hemisphere */

	,
	E_VP_HEM_MAX
} eViperHemisphere;

/////////////////////////////////////////////////////////////////////
// RS-422 Serial configuration parameters
/////////////////////////////////////////////////////////////////////
//! @enum eBaud
//! \anchor anchoreBaud
//! RS-422 Baudrate values used by @ref SERIAL_CONFIG::baudrate
typedef enum eBaud
{
	E_BR_1200 = 0 //!< not used...
	,
	E_BR_2400 //!< not valid for viper
	,
	E_BR_4800 //!< not valid for viper
	,
	E_BR_9600 //!< not valid for viper
	,
	E_BR_19200 //!< not valid for viper
	,
	E_BR_38400 //!< 38,400 baud
	,
	E_BR_57600 //!< 57,600 baud
	,
	E_BR_115200 //!< 115,200 baud (Default)
	,
	E_BR_230400 //!< 230,400 baud
	,
	E_BR_460800 //!< 460,800 baud
	,
	E_BR_921600 //!< 921,600 baud
	,
	E_BR_1843200 //!< 1,843,200 baud
	,
	E_BR_3686400 //!< 3,686,400 baud
	,
	E_BR_7372800 //!< 7,372,800 baud

	,
	E_BR_MAX
} eBaud;

//! @enum eParity
//! \anchor anchoreParity
//! Valid Parity values used by @ref SERIAL_CONFIG::parity
typedef enum eParity
{
	E_PARITY_NONE //!< No parity
	,
	E_PARITY_ODD //!< Odd parity
	,
	E_PARITY_EVEN //!< Even parity

	,
	E_PARITY_MAX
} eParity;

//! @enum eBITcode
//! \anchor anchoreBITcode
//! Valid Built-In Test result codes reflected @ref BIT_STRUCT
typedef enum eBITcode
{
	E_BITERR_NONE //!< No Error.  Self-test and startup successful.
	,
	E_BITERR_SENSOR //!< SENSOR Error
	,
	E_BITERR_SENSOR_PORT //!< SENSOR PORT Error
	,
	E_BITERR_SOURCE //!< SOURCE Error (could also indicate a source port error)
	,
	E_BITERR_SEU //!< SEU Error
	,
	E_BITERR_DUPFREQ //!< Duplicate Source Frequencies Detected.
	,
	E_BITERR_RESERVED1 //!< Reserved.
	,
	E_BITERR_RESERVED2 //!< Reserved.

	,
	E_BITERR_MAX
} eBITcode;

/*
 *  Used to specify the floor compensation level. Floor compensation is used in
 *  source configuration, on a per-source level and are selected based on the distance
 *  in centimeters from the floor.
 */
typedef enum eViperFloorLevels
{
	FLOOR_LVL_NONE = 0 /**< @brief no floor compensation */
	,
	FLOOR_LVL_50 /**< @brief source 50cm off floor */
	,
	FLOOR_LVL_75 /**< @brief source 75cm off floor */
	,
	FLOOR_LVL_100 /**< @brief source 100cm off floor */

	,
	FLOOR_LVL_MAX
} eViperFloorLevels;

//! Command frame preamble. All command frames begin with 'V''P''R''C'.
#define VIPER_CMD_PREAMBLE 0x43525056

//! Position and orientation (PNO) frame preamble. All PNO frames begin with 'V''P''R''P'.
#define VIPER_PNO_PREAMBLE 0x50525056

//! The first 64 bytes of all frames to/from the device.<br>
//! Contents of frame header indicate type and size of frame.
/**
 * 'VPRP' indicates a P&O frame, 'VPRC' indicates a command frame
 *  byte index | name   --| format   | description
 *  -----------|----------|----------| ---------------------------------------------------- -
 * 0-3         | preamble | uint32_t | 'VPRP' indicates a P&O frame, 'VPRC' indicates a command frame
 * 4-7         | size     | uint32_t | count of remaining bytes in frame, including terminal 32-bit CRC
 */
typedef struct _VP_FRAME_HDR
{
	uint32_t preamble; ///< 4 byte preamble @ref VIPER_CMD_PREAMBLE or VIPER_PNO_PREAMBLE indicates frame type.
	uint32_t size;	   ///< 32-bit frame size indicates the number of bytes to follow, including terminating CRC.
} VP_FRAME_HDR;

//! All frames are terminated by a 32-bit calculated CRC value.
typedef uint32_t VPCRC;

/**
 * Every command frame to and from the VIPER system is composed of the following:
 *
 *	VP_FRAME_HDR			: 8 bytes indicates that it is COMMAND frame and specifies size
 *	SEU_CMD					: 20 bytes define the command, action, arguments.
 *	{payload structure}		: optional PAYLOAD structure. Type and size depends on the command
 *	VPCRC					: 32-bit CRC footer calculated on the entire frame
 *
 * Every P&O frame received from the VIPER system is composed of the following:
 *
 *	VP_FRAME_HDR				: 8 bytes indicates that it is PNO frame and specifies size
 *	SEU_PNO						: 16 bytes define the p&o content, including how many sensors reporting.
 *	SENFRAMEDATA[sensorcount]	: 32 byte P&O data block for each reporting sensor
 *	VPCRC						: 32-bit CRC footer calculated on the entire frame
 *
 **/

//! Viper Command frame body definition.
//! Followed by command payload, if any, and always a 32-bit CRC.
/**  byte index | name   | format      | description
 *  -----------|--------|-------------|-----------------------------------------------------
 *  0-3        | SEUid  | uint32_t    | The SEU id. A value of minus 1 represents any or all SEUs.
 *  4-7        | cmd    | uint32_t    | A constant enumeration of type @ref eViperCmds.
 *  8-11       | action | uint32_t    | A constant enumeration of type @ref eCmdActions.
 *  12-15      | arg1   | uint32_t    | Command specific argument.
 *  16-19      | arg2   | uint32_t    | Command specific argument.
 *  Size: 20
 **/
typedef struct _SEUCMD
{
	uint32_t seuid;
	uint32_t cmd;
	uint32_t action;
	uint32_t arg1;
	uint32_t arg2;
} SEUCMD;

//! Command frame header definition.
//! All Viper I/O frames begin with a @ref VP_FRAME_HDR consisting of a preamble and a size field.
//! Command frames follow this with a @ref SEUCMD, an optional payload structure and a 32-bit CRC.
/**  byte index | name     | format    | description
 *  -----------|----------|-----------|-----------------------------------------------
 *  0-3        | preamble | uint32_t  | A constant of value VIPER_CMD_PREAMBLE.
 *  4-7        | size     | uint32_t  | Total number of bytes to follow.
 *  8-27       | SEUcmd   | struct    | See SEUCMD.
 *  Size: 28 bytes
 */
typedef struct _SEUCMD_HDR
{
	uint32_t preamble;
	uint32_t size;
	SEUCMD seucmd; // 20 bytes
} SEUCMD_HDR;

//! SEU-level PNO data info 32-bit bitfield.
/**
 *  byte index | name        | format        | description
 *  -----------|-------------|---------------|-----------------------------------------------
 *   0         | bfPnoMod    | uint32_t : 4  | Type of Pno data in this frame. Enumeration of type @ref ePnoMode.
 *   0         | bfBITerr    | uint32_t : 4  | Non-zero here indicates BIT error detected on system.
 *   1         | bfReserved1 | uint32_t : 8  | Reserved.
 *   2-3       | bfReserved2 | uint32_t : 16 | Reserved.
 */
typedef struct _HPINFO
{
	uint32_t bfPnoMode : 4;
	uint32_t bfBITerr : 4;
	uint32_t bfReserved1 : 8;
	uint32_t bfReserved2 : 16;
} HPINFO;

//! Viper P&O frame body definition.
/**
 * Preceded by @ref VP_FRAME_HDR with @ref VIPER_PNO_PREAMBLE
 * Followed by a payload of @e sensorCount number of @ref SENFRAMEDATA structs and a 32-bit CRC.
 *
 *  byte index | name        | format      | description
 *  -----------|-------------|----------- -|-----------------------------------------------------
 *  0-3        | seuid       | uint32_t    | The seu id. This ID will never be -1.
 *  4-7        | frame       | uint32_t    | The frame number.
 *  8-11       | HPinfo      | bitfield    | Digital I/O information and Source Lock information. See HPINFO.
 *  12-15      | sensorCount | uint32_t    | The number of SENFRAMEDATA structures to follow.
 *  Size: 16 bytes
 */
typedef struct _SEUPNO
{
	uint32_t seuid;
	uint32_t frame;
	HPINFO HPinfo; // 4 bytes
	uint32_t sensorCount;
} SEUPNO;

//! P&O frame header definition.
//! All Viper I/O frames begin with a @ref VP_FRAME_HDR consisting of a preamble and a size field.
//! P&O frames follow this with a @ref SEUPNO, a payload of @ref SENFRAMEDATA structs and a 32-bit CRC.
/**
 *  byte index | name     | format     | description
 *  -----------|----------|------------|-----------------------------------------------
 *  0-3        | preamble | uint32_t   | A constant of value VIPER_PNO_PREAMBLE.
 *  4-7        | size     | uint32_t   | Total number of bytes to follow, including CRC.
 *  8-23       | seudata  | struct     | See SEUPNO.
 *  Size: 24 bytes
 */
typedef struct _SEUPNO_HDR
{
	uint32_t preamble;
	uint32_t size;
	SEUPNO seupno; // 16 bytes
} SEUPNO_HDR;

//! Sensor-level PNO data info 32-bit bitfield.
/**
 *  Each sensor has its own number and they all share the same units. This structure specifies
 *  both values.
 *  byte index | name         | format          | description
 *  -----------|--------------|-----------------|-----------------------------------------------
 *   0         | bfSnum       | uint32_t : 7    | The 0-based sensor number.
 *   0         | bfSvirt      | uint32_t : 1    | Bit set to 1 if sensor is virtual
 *   1         | bfPosUnits   | uint32_t : 2    | A constant enumeration of type @ref eViperPosUnits.
 *   1         | bfOriUnits   | uint32_t : 2    | A constant enumeration of type @ref eViperOriUnits.
 *   2         | bfBtnState0  | uint32_t : 1    | Bitmap state for button 0 on sensor.
 *   2         | bfBtnState1  | uint32_t : 1    | Bitmap state for button 1 on sensor.
 *   2         | bfDistortion | uint32_t : 8    | Distortion detected level 0-255.
 *   2-3       | bfAuxInput   | uint32_t : 10   | Reserved for auxilliary and custom inputs.
 */
typedef struct _SFINFO
{
	uint32_t bfSnum : 7;
	uint32_t bfSvirt : 1;
	uint32_t bfPosUnits : 2;
	uint32_t bfOriUnits : 2;
	uint32_t bfBtnState0 : 1;
	uint32_t bfBtnState1 : 1;
	uint32_t bfDistortion : 8;
	uint32_t bfAuxInput : 10;
} SFINFO;

//! Sensor position and orientation data format.
/**
 *  If orientation units are Euler, then the last 4 bytes are not used.
 *  byte index | name       | format    | description
 *  -----------|------------|---------- |----------------------------------
 *   0-3       | pos[0]     | float     | Position x.
 *   4-7       | pos[1]     | float     | Position y.
 *   8-11      | pos[2]     | float     | Position z.
 *   12-15     | ori[0]     | float     | Euler azimuth   or Quaternion w (spin)
 *   16-19     | ori[1]     | float     | Euler elevation or Quaternion x
 *   20-23     | ori[2]     | float     | Euler roll      or Quaternion y
 *   24-27     | ori[3]     | float     | N/A (zero)      or Quaternion z
 */
typedef struct _PNO
{
	float pos[3];
	float ori[4];
} PNODATA;

//! Sensor position and orientation data format with acceleration.
/**
 *  If orientation units are Euler, then bytes 18-19 are not used.
 *  Orientation and Acceleration values are compressed into 16-bit Q15 fractional integers
 *  Conversion of fractional fields back to floats may be accomplished via @ref FractToFloat function
 *  using FFACTOR_EULER_DEGREE, _EULER_RAD, _QTRN or _ACCEL factors.
 *  byte index | name       | format    | description
 *  -----------|------------|---------- |----------------------------------
 *   0-3       | pos[0]     | float     | Position x.
 *   4-7       | pos[1]     | float     | Position y.
 *   8-11      | pos[2]     | float     | Position z.
 *   12-13     | ori[0]     | int16     | Q15 Fractional Euler azimuth   or Quaternion w (spin)
 *   14-15     | ori[1]     | int16     | Q15 Fractional Euler elevation or Quaternion x
 *   16-17     | ori[2]     | int16     | Q15 Fractional Euler roll      or Quaternion y
 *   18-19     | ori[3]     | int16     | N/A (zero)                     or Quaternion z
 *   20-21     | acc[0]     | int16     | Q15 Fractional Acceleration in X
 *   22-23     | acc[1]     | int16     | Q15 Fractional Acceleration in Y
 *   24-25     | acc[2]     | int16     | Q15 Fractional Acceleration in Z
 *   26-27     | acc[3]     | int16     | Q15 Fractional Acceleration Magnitude in XYZ
 */
typedef struct _PNO_A
{
	float pos[3];
	int16_t ori[4];
	int16_t acc[4];
} PNO_A;

//! Sensor payload of SEU P&O frame @ref SEUPNO.
/**
 *  This is the payload of a PNO frame. Each frame can have 0-16 sensor's worth of data and is therefore variable in size:
 *  the size is 32 bytes (this accounts for the header at the beginning and CRC at the end) plus a multiple of 32 bytes
 *  times the number of active sensors.
 *  byte index | name        | format         | description
 *  -----------|-------------|----------------|-----------------------------------------------
 *   0-3       | SFinfo      | bitfield       | Contains sensor information and pno UNITS information. See SFINFO.
 *   4-31      | pno         | structure      | Contains the sensors position and orientation. See PNODATA.
 *  Size: 32 bytes.
 */
typedef struct _SENFRAMEDATA
{

	SFINFO SFinfo; // 4 bytes
	PNODATA pno;   // 28 bytes
} SENFRAMEDATA;

//! Sensor payload with Acceleration of SEU P&O frame @ref SEUPNO.
/**
 *  This is the payload of a PNO_A frame containing acceleration.
 *  Each frame can have 0-16 sensor's worth of data and is therefore variable in size:
 *  the size is 32 bytes (this accounts for the header at the beginning and CRC at the end) plus a multiple of 32 bytes
 *  times the number of active sensors.
 *  byte index | name        | format         | description
 *  -----------|-------------|----------------|-----------------------------------------------
 *   0-3       | SFinfo      | bitfield       | Contains sensor information and pno UNITS information. See SFINFO.
 *   4-31      | pno_a       | structure      | Contains the sensors position, orientation and acceleration. See PNOA.
 *  Size: 32 bytes.
 */
typedef struct _SENFRAMEDATA_A
{

	SFINFO SFinfo; // 4 bytes
	PNO_A pno;	   // 28 bytes
} SENFRAMEDATA_A;

#define NAME_SIZE 64   //!< Number of WhoAmI device_name ASCII characters: 64
#define SERNUM_SIZE 32 //!< Number of WhoAmI serial number ASCII characters: 32
#define PN_SIZE 32	   //!< Number of WhoAmI part number ASCII characters: 32

//!  Payload for @ref CMD_WHOAMI command.<BR>
//!  Used to query for device information. <BR>
//!  All fields expressed as ASCII character strings with no null termination. <BR>
/**
 *  byte index | name           | format     | description
 *  -----------|----------------|------------|-------------------------------------------
 *  0-63       | device_name    | char[]     | The product name. 64 chars
 *  64-95      | hw_ser_no      | char[]     | Serial number of the device. 32 chars
 *  96-127     | ioproc_pn      | char[]     | The ioproc firmware part number. 32 chars
 *  128-159    | dsp_bt_fw_pn   | char[]     | The boot loader firmware part number. 32 chars
 *  160-191    | dsp_app_fw_pn  | char[]     | The application firmware part number. 32 chars
 */
typedef struct _WHOAMI_STRUCT
{
	char device_name[NAME_SIZE]; //!< Product name
	char hw_ser_no[SERNUM_SIZE]; //!< Hardware serial number
	char ioproc_pn[PN_SIZE];	 //!< IOProc part number
	char dsp_bt_fw_pn[PN_SIZE];	 //!< Boot firmware part number
	char dsp_app_fw_pn[PN_SIZE]; //!< Main firmware part number
} WHOAMI_STRUCT;

//! Payload for @ref CMD_SEUID command. <BR>
//! 32-bit numeric SEU identification
/**. No two SEUs, when used together, should
 * have the same number. The only value reserved is -1, however typically values under 5000
 * are encouraged. Values with length less than or equal to five digits are strongly encouraged.
 */
typedef uint32_t VIPER_SEUID;

typedef struct _SEUID_CONFIG
{
	uint32_t seuid;
} SEUID_CONFIG;

//! Payload for @ref CMD_UNITS command. <br>
//! Defines tracker output Position and Orientation units of measure.<br>
/**
 *  Valid values defined in @ref eViperPosUnits and @ref eViperOriUnits.<br>
 *  Used to configure seu units. This affects both the command frames (commands sent to the seu must
 *  use these units) and the PNO frames, whose data will appear in these units.
 *  byte index | name           | format     | description
 *  -----------|----------------|------------|------------------------------------------------
 *  0-3        | pos_units      | uint32_t   | A constant enumeration of type @ref eViperPosUnits.
 *  4-7        | ori_units      | uint32_t   | A constant enumeration of type @ref eViperOriUnits.
 */
typedef struct _UNITS_CONFIG
{
	uint32_t pos_units; // valid values in @ref eViperPosUnits
	uint32_t ori_units; // valid values in @ref eViperOriUnits
} UNITS_CONFIG;

//! Payload for @ref CMD_STATION_MAP command..<br>
//! Bitmaps indicate detected sensors and sources. (READ-ONLY)<br>
/**
 *  byte index | name         | format         | description
 *  -----------|--------------|----------------|------------------------------------------------
 *  0-1        | sensor_map   | uint32_t : 16  | Bitmap of detected sensors across all tracker boards
 *  2          | reserved1    | uint32_t : 8   | reserved
 *  3:0-3      | source_map   | uint32_t : 4   | Bitmap of detected sources across all tracker boards
 *  3:4-7      | reserved2    | uint32_t : 4   | reserved
 */
typedef struct _STATION_MAP
{
	union
	{
		uint32_t stamap;
		struct
		{
			uint32_t sensor_map : 16;
			uint32_t reserved1 : 8;
			uint32_t source_map : 4;
			uint32_t reserved2 : 4;
		} bf;
	};
} STATION_MAP;

//! Payload for @ref CMD_BORESIGHT command. <br>
/**
*  Used to configure sensor @ref boredefn.
*  Sensors are boresighted independent of each other.
*  The effect of boresighting a Sensor is that orientation outputs for the Sensor will equal
*  the specified boresight reference angles at the current physical orientation of the Sensor hardware.

*  When used with the GET action, the boresight payload contains the rotation applied to produce
*  the boresight, *not* the original reference angles specified with the SET action.  If (0.0, 0.0, 0.0)
*  is the result of a GET boresight action, then no boresight is applied to the sensor.
*
*  Angles in this payload are expressed in the units configured by @ref CMD_UNITS command.
*
*  byte index | name           | format    | description
*  -----------|----------------|-----------|------------
*  0-3        | params[0]      | float     | Euler azimuth   or Quaternion w (spin)
*  4-7        | params[1]      | float     | Euler elevation or Quaternion x
*  8-11       | params[2]      | float     | Euler roll      or Quaternion y
*  12-15      | params[3]      | float     | N/A (zero)      or Quaternion z
*/
typedef struct _BORESIGHT_CONFIG
{
	float params[4];
} BORESIGHT_CONFIG;

//! Payload for @ref CMD_SRC_ROTATION command.
/**
*  Used to configure the source rotation.
hex(msg.to)
hex(msg.to)
*  Angles in this payload are expressed in the units configured by @ref CMD_UNITS.
*
*  byte index | name           | format      | description
*  -----------|----------------|-------------|------------
*  0-3        | srcfreq        | uint32_t    | frequency index (read-only) for @ref CMD_SRC_ROTATION command.
*  4-7        | rot[0]         | float       | Euler azimuth   or Quaternion w (spin)
*  8-11       | rot[1]         | float       | Euler elevation or Quaternion x
*  12-15      | rot[2]         | float       | Euler roll      or Quaternion y
*  16-19      | rot[3]         | float       | N/A (zero)      or Quaternion z
*/
typedef struct _SRCROT_CONFIG
{
	uint32_t srcfreq;
	float rot[4];
} SRCROT_CONFIG;

//! Payload for @ref CMD_SOURCE_CFG command
/**
 *  Used for extended range multi=source configuration.
 *  Position and angles in this payload are expressed in the units configured by CMD_UNITS.
 *  Position relative to reference position determined by end user
 *
 *  byte index | name           | format         | description
 *  -----------|----------------|----------------|------------
 *  0-3        | srcfreq        | 32-bit unsignd | source frequency (READ ONLY)
 *  4-31       | srcpno         | structure      | source position and orientation relative to virtual reference.
 *  32-35      | startuphem     | uint32_t       | A constant enumeration of type @ref eViperHemisphere.
 */
typedef struct _SRC_CONFIG
{
	uint32_t srcfreq;
	PNODATA srcpno;
	uint32_t startuphem; // valid values in @ref eViperHemisphere
} SRC_CONFIG;

//! Payload for @ref CMD_PREDFILTER_CFG command
/**
 *  byte index | name           | format         | description
 *  -----------|----------------|----------------|------------
 *  0-3        | qFil_on        | 32-bit unsignd | Orientation Predition Filter Enabled 0/1. Treated as a binary value.
 *  4-7        | rFil_on        | 32-bit unsignd | Position Prediction Filter Enabled 0/1. Treated as a binary value.
 *  8-11       | predTimeS      | float          | Prediction time in seconds. Default: 20ms, 0.02sec
 */
typedef struct _PF_CONFIG
{
	uint32_t qFil_on; // quat prediction on/off
	uint32_t rFil_on; // pos prediction on/off
	float predTimeS;  // prediction time seconds Default: 20ms, 0.02sec
} PF_CONFIG;

typedef struct _NOISEPROFILE
{
	float Rq[4][4]; // dq measurment noise
	float Qq[3][3]; // dq process noise
	float Rr[3][3]; // dr measurment noise
	float Qr[3][3]; // dr process noise
} NOISEPROFILE;

//! Payload for @ref CMD_PREDFILTER_EXT command
/**
 *  Extended Prediction Filter info.
 *
 *  byte index | name           | format         | description
 *  -----------|----------------|----------------|------------
 *  0-3        | qFil_on        | 32-bit unsignd | Orientation Predition Filter Enabled 0/1. Treated as a binary value.
 *  4-7        | rFil_on        | 32-bit unsignd | Position Prediction Filter Enabled 0/1. Treated as a binary value.
 *  8-11       | predTimeS      | 32-bit unsignd | Prediction time in seconds. Default 0.02 sec, 20ms
 *  12-15      | res            | float          | Internal use only
 *  16-79      | nprof.Rq       | float[4][4]    | dq measurement noise
 *  80-115     | nprof.Qq       | float[3][3]    | dq process noise
 *  116-151    | nprof.Rr       | float[3][3]    | dr measurement noise
 *  152-187    | nprof.Qr       | float[3][3]    | dr process noise
 *
 */
typedef struct _PF_CONFIG_EXT
{
	uint32_t qFil_on; // quat prediction on/off
	uint32_t rFil_on; // pos prediction on/off
	float predTimeS;  // prediction time seconds
	float res;		  // internal use only
	NOISEPROFILE nprof;
} PF_CONFIG_EXT;

//! Payload for @ref CMD_TIP_OFFSET command.
/**
 *  Used to configure sensor @ref tipoffdefn. Each sensor can have a different offset.
 *  Once set, all future PNO will rotate this vector into the sensors frame of reference
 *  and add it to the sensors position.
 *  Offsets expressed in position units configured by @ref CMD_UNITS.
 *  byte index | name           | format   | description
 *  -----------|----------------|----------|------------
 *  0-3        | params[0]      | float    | Position X-coordinate offset.
 *  4-7        | params[1]      | float    | Position y-coordinate offset.
 *  8-11       | params[2]      | float    | Position z-coordinate offset.
 */
typedef struct _TIP_OFFSET_CONFIG
{
	float params[3];
} TIP_OFFSET_CONFIG;

//! Payload for @ref CMD_HEMISPHERE command. <br>
/**
 *  Used to configure sensor @ref hemdefn. Each sensor can have a different hemisphere.
 *  byte index | name           | format        | description
 *  -----------|----------------|---------------|------------
 *  0          | track_en       | uint32_t : 1  | Hemisphere Tracking Enabled 0/1. Treated as a binary value.
 *  0          | auto_en        | uint32_t : 1  | Auto-Hemisphere enabled 0/1
 *  1-3        | res            | uint32_t : 30 | reserved
 *  4-7        | params[0]      | float         | X-coordinate of vector describing Hemisphere.
 *  8-11       | params[1]      | float         | Y-coordinate of vector describing Hemisphere.
 *  12-15      | params[2]      | float         | Z-coordinate of vector describing Hemisphere.
 */
typedef struct _HEMISPHERE_CONFIG
{
	struct
	{
		uint32_t track_en : 1;
		uint32_t auto_en : 1;
		uint32_t res : 30;
	} bf;
	float params[3];
} HEMISPHERE_CONFIG;

//! Payload for @ref CMD_INCREMENT command. <br>
/**
 *  Used to configure sensor @ref incrdefn. Each sensor can have a different increment.
 *  When enabled, position and orientation values will move in steps specified by the position
 *  and orientation threshold.
 *  Thresholds are expressed in position and orientation units configured by @ref CMD_UNITS.
 *  Threshold values less than or equal to 0.0 enable Auto-Increment feature.
 *  Auto-Increment: When enabled, new Position or Orientation data are reported only when changes have
 *  satisfied dynamic internal predefined criteria.  Criteria depend on EM signal strength and noise
 *  values.
 *  byte index | name           | format   | description
 *  -----------|----------------|----------|---------------------------
 *  0-3        | enabled        | uint32_t | Increment feature enabled or not.  O: Not enabled, Non-zero: Enabled.
 *  4-7        | fPosThresh     | float    | The position threshold, expressed in system configured UNITS.
 *  8-11       | fOriThresh     |278 float    | The orientation threshold, expressed in Euler Degrees.
 */
typedef struct _INCREMENT_CONFIG
{
	uint32_t enabled; // 0: enabled, non-zero: enabled
	float fPosThresh; // <= 0.0f : Auto
	float fOriThresh; // <= 0.0f : Auto
} INCREMENT_CONFIG;

//! Payload for @ref CMD_FILTER command. <br>
/**
 *  Used to configure @ref filterdefn. <br>
 *  Each sensor can have a different filter level. <br<
 *  Further, position and orientation can have different filter levels.
 *  Once set, the filter is applied to all future position and orientation,
 *  smoothing the results at the cost of a higher perceived latency.
 *  If field @e level is set to value @ref FILTER_LVL_CUSTOM, then custom values in
 *  @e params[] fields are applied.
 *  byte index | name      | format    | description
 *  -----------|-----------|-----------|---------------------------------------------------
 *  0-3        | level     | uint32_t  | A constant enumeration of type @ref eViperFilterLevel.
 *  4-7        | params[0] | float     | F : Sensitivity.								Range 0 < F < 1.0
 *  8-11       | params[1] | float     | FLow : static max filter 1.0 disables filter.	Range 0 < FLow < FHigh
 *  12-15      | params[2] | float     | FHigh : dynamic min filter.						Range FLoW < FHigh < 1.0
 *  16-19      | params[3] | float     | MAXT : Max transition rate min->max filtering.	Range 0 < MAXT < 1.0
 */
typedef struct _FILTER_CONFIG
{
	uint32_t level;
	float params[4];
} FILTER_CONFIG;

//! Payload for @ref CMD_SRC_SELECT
/**
 *  Used to configure sensor source selection.
 *  Selected sources are used by sensor in calculating P&O.
 *  Selected sources are expressed as a bitmap of sources 0-(SOURCES_PER_SEU-1) (maximum 4)
 *  If bitmap is zero, then the sensor uses only the source that is LOCAL to it.
 *  The LOCAL source is plugged into the same bank of connectors as the sensor.
 *  That is:
 *     Source 0 is LOCAL to sensors 0-3
 *     Source 1 is LOCAL to sensors 4-7
 *     Source 2 is LOCAL to sensors 8-11
 *     Source 3 is LOCAL to sensors 12-15.
 *  Default: LOCAL (zero).
 *
 *  byte index | name        | format    | description
 *  -----------|-------------|-----------|---------------------------------------------------
 *  0-3        | uSrcSelMap  | int32_t   | Bitmap of sources used by sensor in calculating p&o.  Default: 0.
 */
typedef struct _SRC_SEL_CONFIG
{
	uint32_t uSrcSelMap;
} SRC_SEL_CONFIG;

//! Payload for @ref CMD_SNS_VIRTUAL
/**
 *  Used to configure virtual sensors
 *  Virtual sensors are virtual "replicas" of a selected active "feeder" sensor.
 *  The virtual sensor P&O is captured from the active sensor.  That is, the active sensor "feeds" the virtual sensor.
 *  Sensor configuration settings may be applied independently to virtual sensors so that
 *  virtual sensor output is the actual sensor's unconfigured (raw) data with virtual sensor settings applied.
 *  Virtual-Feeder sensor pairs are limited to sensor numbers on the same sensor board (bank of sensor connectors).
 *  That is:
 *     Sensor nums 0-3 may be virtualized, and/or
 *     Sensor nums 4-7, and/or
 *     Sensor nums 8-11, and/or
 *     Sensor nums 12-15.
 *  Only active (connected) sensors may "feed" virtual sensor output.
 *  Actual sensors may "feed" up to three virtual sensors.
 *  Virtual sensors may not "feed" other virtual sensors.
 *  Virtual sensors are created by sending the CMD_SNS_VIRTUAL CMD_ACTION_SET to the virtual sensor number.
 *  Virtual sensors are destroyed by sending CMD_SNS_VIRTUAL CMD_ACTION_RESET to the virtual sensor number.
 *
 *  byte index | name        | format     | description
 *  -----------|-------------|------------|---------------------------------------------------
 *  0-3        | uActiveSns  | uint32_t   | Bitmap of sources used by sensor in calculating p&o.
 *  4-7        | reserved0   | uint32_t   | reserved
 *  8-11       | reserved1   | uint32_t   | reserved
 *  12-15      | reserved2   | uint32_t   | reserved
 */
typedef struct _VIRTUAL_SNS_CONFIG
{
	uint32_t uInput1;
	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t reserved2;
} VIRTUAL_SNS_CONFIG;

//! @typedef ENUM_CONFIG
//! Used as command payload for config settings that are enumerated values, eg @ref CMD_FRAMERATE, @ref CMD_FTT_MODE, @ref CMD_STYLUS.
//! /anchor anchorENUM_CONFIG
typedef uint32_t ENUM_CONFIG;

//! @typedef BINARY_CONFIG
//! Payload for BINARY COMMANDS.<br>
//! /anchor anchorBINARY_CONFIG
//! Used as command payload for config settings that have only two states: on or off.<br>
//! E.g. @ref CMD_SYNC_MODE, @ref CMD_DUAL_OUTPUT.
/**  Valid values are:
 * 0 : false / OFF
 * Non-zero : true / ON
 */
typedef int32_t BINARY_CONFIG;

//! @typedef SNS_ORIG_CONFIG
//! Payload for @ref CMD_SNS_ORIGIN command.<br>
//! Valid values defined in @ref eSensorOrigin
typedef ENUM_CONFIG SNS_ORIG_CONFIG;

//! @typedef FRAMERATE_CONFIG
//! Payload for @ref CMD_FRAMERATE command.<br>
//! /anchor anchorFRAMERATE_CONFIG
//! Valid values defined in @ref eViperFrameRate
typedef ENUM_CONFIG FRAMERATE_CONFIG;

//! @typedef FTTMODE_CONFIG
//! Payload for @ref CMD_FTT_MODE command. <br>
//! Used to configure FTT Mode. Valid values defined by @ref eFTTMode.
typedef ENUM_CONFIG FTTMODE_CONFIG; //!< valid values in @ref eFTTMode

//! @typedef STYLUS_CONFIG
//! Payload for @ref CMD_STYLUS command.<>
//! Used to configure stylus mode of operation. Valid valued defined in @ref eStylusMode.
typedef ENUM_CONFIG STYLUS_CONFIG; //!< valid values in @ref eStylusMode

//! Payload for @ref CMD_SERIAL_CONFIG command.<br>
//! Used to configure serial port settings for RS-422 interface.
/**
 * Struct does not contain fields for data bits or handshake:  <br>
 * For Viper these are not-configurable: 8 data bits and No Handshake.
 *  byte index | name      | format        | description
 *  -----------|-----------|---------------|---------------------------------------------------
 *  0-3        | sercfg    | uint32_t      | 32-bit union of other fields
 *  0:4        | baudrate  | uint32_t : 4  | Uses enum values of type @ref eBaud
 *  0:2        | parity    | uint32_t : 2  | Uses enum values of type @ref eParity
 *  0-3        | res       | uint32_t : 26 | reserved
 */
typedef struct _SERIAL_CONFIG
{
	union //!< 32-bit union of serial port settings
	{
		uint32_t sercfg; //!< 32-bit union of serial port settings
		struct _bf
		{						   //!< bitfield representation of RS422 settings
			uint32_t baudrate : 4; //!< uses values from @ref eBaud
			uint32_t parity : 2;   //!< uses values from @ref eParity
			uint32_t res : 26;
		} bf;
	};
} SERIAL_CONFIG;

// payload for GET CMD_BIT
//! Payload for @ref CMD_BIT command.<br>
/**
 *  byte index | name            | format         | description
 *  -----------|-----------------|----------------|---------------------------------------------------
 *  0-1        | bfSensorErr     | uint32_t : 16  | Bitmap of detected E_BITERR_SENSOR across all tracker boards
 *  2-3        | res1            | uint32_t : 16  | reserved
 *  4-5        | bfSensorPortErr | uint32_t : 16  | Bitmap of detected E_BITERR_SENSOR_PORT across all tracker boards
 *  6-7        | res2            | uint32_t : 16  | reserved
 *  8:0-3      | bfSourceErr     | uint32_t : 4   | Bitmap of detected E_BITERR_SOURCE across all tracker boards
 *  8:4-7      | res3            | uint32_t : 4   | reserved
 *  9-11       | res4            | uint32_t : 24  | reserved
 *  12:0-3     | bfSEUErr        | uint32_t : 4   | Bitmap of detected E_BITERR_SEU across all tracker boards
 *  12:4-7     | res5            | uint32_t : 4   | reserved
 *  13-15      | res4            | uint32_t : 24  | reserved
 *  16:0-3     | bfDupFreqErr    | uint32_t : 4   | Bitmap of detected E_BITERR_DUPFREQ across all tracker boards
 *  16:4-7     | res5            | uint32_t : 4   | reserved
 *  17-19      | res4            | uint32_t : 24  | reserved
 *  20-23      | bfReserved1     | uint32_t       | Reserved
 *  24-27      | bfReserved2     | uint32_t       | Reserved
 */

typedef struct _BIT_STRUCT
{
	uint32_t bfSensorErr : 16; //<! a 1 in any bit shows E_BITERR_SENSOR
	uint32_t res1 : 16;
	uint32_t bfSensorPortErr : 16; //<! a 1 in any bit shows E_BITERR_SENSOR_PORT
	uint32_t res2 : 16;
	uint32_t bfSourceErr : 4; //<! a 1 in any bit shows E_BITERR_SOURCE
	uint32_t res3 : 4;
	uint32_t res4 : 24;
	uint32_t bfSEUErr : 4; //<! a 1 in any bit shows sensor board E_BITERR_SEU
	uint32_t res5 : 4;
	uint32_t res6 : 24;
	uint32_t bfDupFreqErr : 4; //<! a 1 in any bit shows sources with same freq (E_BITERR_DUPFREQ)
	uint32_t res7 : 4;
	uint32_t res8 : 24;
	uint32_t bfReserved1 : 32;
	uint32_t bfReserved2 : 32;
} BIT_STRUCT;

/*
 * Used to specify FrameCount behavior when starting continuous mode with CMD_CONTINUOUS_PNO CMD_ACTION_SET
 * valid values are
 *  -1  : Leaves FC unchanged
 *   0  : Resets FC to 0
 *   all others : Sets FC to specified value
 */
typedef int32_t CONT_FC_CONFIG;

/*
 *  Provides more details about a map.
 *  byte index | name           | format       | description
 *  -----------|----------------|--------------|---------------------------------------------
 *  0-47       | map_name       | char[]       | The ASCII name of the map. Not null-terminated.
 *  48-51      | qty            | uint32_t     | The number of sources in this map.
 *  52-63      | resv           | uint32_t[]   | Reserved.
 */
typedef struct MAP_INFO_CONFIG
{
	char map_name[48];
	uint32_t qty;
	uint32_t resv[3];
} MAP_INFO_CONFIG;

/*
 *  Comp2 map coefficients
 *  byte index | name                | format    | description
 *  -----------|---------------------|-----------|------------------------------------------------------------------------
 *  0-3        | source_id           | uint32_t  | Frequency identifier of the source that the map applies to
 *  4-983      | coeffs              | float[][] | 35 x 7 array of correction coefficients
 */
typedef struct COMP2_PAYLOAD
{
	uint32_t source_id;
	float coeffs[35][7];
} COMP2_PAYLOAD;

//! The payload of a @ref CMD_ACTION_NAK received from Viper SEU.
/**
 *  byte index | name   | format      | description
 *  -----------|--------|-------------|------------------------------------------------------------------------
 *  0-3        | code   | uint32_t    | Device specific error code, valid values in @ref eNakType
 *  4-131      | text   | char[]      | ASCII Description of the error, not null terminated.
 *  132-179    | data   | uint32_t[]  | Any other data related to the error.
 */
typedef enum
{
	E_NAK_TYPE_GENERAL //<! General purpose NAK.  Check Text &/or Data fields for detail, if any
	,
	E_NAK_TYPE_NOT_READY //<! Tracker not ready, booting up.
	,
	E_NAK_TYPE_CMDERR //<! Command execution error: could be an FTT command applied to a non-FT sensor
	,
	E_NAK_TYPE_BITERR //<! BIT error condition prevent command execution
	,
	E_NAK_TYPE_RESERVED

	,
	E_NAK_TYPE_MAX
} eNakType;
typedef struct _NAK_INFO
{
	uint32_t code; // valid values in @ref eNakType
	char text[128];
	uint32_t data[12]; // contains BIT_STRUCT if code == E_NAK_TYPE_BITERR
} NAK_INFO;

/*
 * Allocation of MAXCOMMAND allocates enough memory for any viper command frame
 */
typedef struct _MAXCMDSTRUCT
{
	VP_FRAME_HDR hdr;
	SEUCMD cmd;
	union
	{
		HEMISPHERE_CONFIG hem;
		FILTER_CONFIG filter;
		INCREMENT_CONFIG incr;
		TIP_OFFSET_CONFIG tipoff;
		BORESIGHT_CONFIG bore;
		WHOAMI_STRUCT wai;
		FRAMERATE_CONFIG frate;
		UNITS_CONFIG units;
		SRCROT_CONFIG srcrot;
		BINARY_CONFIG syncmode;
		STATION_MAP stamap;
		STYLUS_CONFIG stylusmode;
		VIPER_SEUID seuid;
		BINARY_CONFIG usbmode;
		SERIAL_CONFIG sercfg;
		BIT_STRUCT biterr;
		CONT_FC_CONFIG contfc;
		FTTMODE_CONFIG fttmode;
		SRC_CONFIG srccfg;
		PF_CONFIG pfcfg;
		PF_CONFIG_EXT pfcfgext;
		SRC_SEL_CONFIG srcsel;
		SNS_ORIG_CONFIG snsorig;
		VIRTUAL_SNS_CONFIG virtsns;
		MAP_INFO_CONFIG mapinfo;
		//		COMP2_PAYLOAD		comp;
		NAK_INFO nak;
	} payload;
	VPCRC crc;
} MAXCOMMAND;

/*
 * Allocation of MAXPNO allocates enough memory for any viper p&o frame
 */
typedef struct _MAXPNOSTRUCT
{
	VP_FRAME_HDR hdr;
	SEUPNO cmd;
	SENFRAMEDATA sens[SENSORS_PER_SEU];
	VPCRC crc;
} MAXPNO;

//! Viper frame CRC Calculation function.
/**
 *  Calculates the CRC (Cyclic Redundancy Check) of I/O frame buffer to/from a Viper SEU.
 *  This form of CRC calculation is used by both SEU firmware and any host application which communicates with Viper.
 *
 *  For host software developers implementing a raw interface directly, this function is defined in ViperInterface.c for reference.
 *  ViperInterface.c may be used directly or the function definition copied and pasted into custom code.
 *
 *  @param data
 *    <br>A pointer to a memory location where data will be read from.
 *  @param count
 *    <br>The number of bytes of data to use in the calculation.
 */
uint32_t Viper_CalcCRC_Bytes(unsigned char *data, uint32_t count);

/**
 *  Caclulates the CRC (Cyclic Redundancy Check) of some buffer from, or be sent to, a Viper seu. This method will
 *  be used by both a seu's firmware and any host application which communicates with a seu.
 *  @param data
 *    <br>A pointer to a memory location where data will be read from.
 *  @param count
 *    <br>The number of bytes of data to use in the calculation.
 */
// uint32_t Viper_CalcCRC_Ints(uint32_t *data, uint32_t count);

//! Q15 Fractional conversion to float function
/**
 * Converts 16-bit Q15 fractional values returned in @ref PNO_A structure into 32-bit floating point.
 * The 16-bit Q15 Fractional number is a 16-bit signed fixed-point fractional value.
 * This Q15 is also known as Q0.15: 0 bits are used for the integer and 15 bits are used for the fraction.
 * For certain DSPs, this is also known as a fract16 number.
 * The MSB of the number is a sign bit.
 * QN is a ratio where the denominator is 2^N.  For Q15, the denominator is 2^15
 * The fraction represents a ratio of the floating point value being transmitted to maximum possible for that value.
 * For example, if the Q15 value represents radians in a elevation, the maximum elevation value is PI.
 * (There are 2*PI radians in a full 360-degree rotation and Viper emits rotation values in the range +/- PI radians.)
 * Thus when converting from the Q15 elevation value to a float, the factor argument will be PI.
 * Factors for output fields using the Q15 format are defined by FFACTOR_EULER_DEGREE, _EULER_RAD, _QTRN, and _ACCEL
 *
 *  @param fract
 *    <br>16-bit Q15 number received from Viper
 *  @param factor
 *    <br>The maximum possible value of floating point result
 */
float FractToFloat(int16_t fract, float factor);
#define FFACTOR_EULER_DEGREE 180.0f
#define FFACTOR_EULER_RAD 3.14159265358979323846f
#define FFACTOR_QTRN 1.0f
#define FFACTOR_ACCEL 16.0f
#define Degree_Fract2Float(i_deg) FractToFloat(i_deg, FFACTOR_EULER_DEGREE);
#define Radian_Fract2Float(i_rad) FractToFloat(i_rad, FFACTOR_EULER_RAD);
#define Quaternion_Fract2Float(i_quat) FractToFloat(i_qt, FFACTOR_QTRN);
#define Acceleration_Fract2Float(i_acc) FractToFloat(i_acc, FFACTOR_ACCEL);

/*
 *  END $Workfile: ViperInterface.h $
 */
#endif /* VIPERINTERFACE_H_INCLUDED */
