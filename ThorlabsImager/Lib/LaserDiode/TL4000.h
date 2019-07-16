/****************************************************************************

   Thorlabs TL4000 VXIpnp instrument driver

   FOR DETAILED DESCRIPTION OF THE DRIVER FUNCTIONS SEE THE ONLINE HELP FILE
   AND THE PROGRAMMERS REFERENCE MANUAL.

   Copyright:  Copyright(c) 2008-2014, Thorlabs (www.thorlabs.com)
   Author:     Michael Biebl (mbiebl@thorlabs.com)

   Disclaimer:

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   Header file

   Date:          Apr-11-2014
   Built with:    NI LabWindows/CVI 13.0.1
   Software-Nr:   09.178.xxx
   Version:       3.1.0

   Changelog:     see 'readme.rtf'

****************************************************************************/


#ifndef _TL4000_DRIVER_HEADER_
#define _TL4000_DRIVER_HEADER_

#include <vpptype.h>

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C"
{
#endif


/*===========================================================================

 Macros

===========================================================================*/
/*---------------------------------------------------------------------------
  USB stuff
---------------------------------------------------------------------------*/
#define TL4000_VID_THORLABS            (0x1313)    // Thorlabs

#define TL4000_PID_TED4000_DFU         (0x8040)    // TED4xxx with DFU interface enabled
#define TL4000_PID_LDC4000_DFU         (0x8041)    // LDC4xxx with DFU interface enabled
#define TL4000_PID_ITC4000_DFU         (0x8042)    // ITC4xxx with DFU interface enabled

#define TL4000_PID_TED4000             (0x8048)    // TED4xxx w/o DFU interface
#define TL4000_PID_LDC4000             (0x8049)    // LDC4xxx w/o DFU interface
#define TL4000_PID_ITC4000             (0x804A)    // ITC4xxx w/o DFU interface

#define TL4000_PID_CLD1000_DFU         (0x8047)    // CLD1xxx with DFU interface enabled
#define TL4000_PID_CLD1000             (0x804F)    // CLD1xxx w/o DFU interface
#define TL4000_PID_GH_EM595_DFU        (0x8046)    // Gooch&Housego EM595 with DFU interface enabled
#define TL4000_PID_GH_EM595            (0x804E)    // Gooch&Housego EM595 w/o DFU interface

/*---------------------------------------------------------------------------
 Find pattern for 'viFindRsrc()'
---------------------------------------------------------------------------*/
#define TL4000_FIND_PATTERN_TED        "USB?*INSTR{VI_ATTR_MANF_ID==0x1313 && (VI_ATTR_MODEL_CODE==0x8040 || VI_ATTR_MODEL_CODE==0x8048)}"
#define TL4000_FIND_PATTERN_LDC        "USB?*INSTR{VI_ATTR_MANF_ID==0x1313 && (VI_ATTR_MODEL_CODE==0x8041 || VI_ATTR_MODEL_CODE==0x8049)}"
#define TL4000_FIND_PATTERN_ITC        "USB?*INSTR{VI_ATTR_MANF_ID==0x1313 && (VI_ATTR_MODEL_CODE==0x8042 || VI_ATTR_MODEL_CODE==0x804A)}"
#define TL4000_FIND_PATTERN_CLD        "USB?*INSTR{VI_ATTR_MANF_ID==0x1313 && (VI_ATTR_MODEL_CODE==0x8047 || VI_ATTR_MODEL_CODE==0x804F || VI_ATTR_MODEL_CODE==0x8046 || VI_ATTR_MODEL_CODE==0x804E)}"

#define TL4000_FIND_PATTERN_ANY4000    "USB?*INSTR{VI_ATTR_MANF_ID==0x1313 && (VI_ATTR_MODEL_CODE==0x8040 || VI_ATTR_MODEL_CODE==0x8041 || VI_ATTR_MODEL_CODE==0x8042 || VI_ATTR_MODEL_CODE==0x8048 || VI_ATTR_MODEL_CODE==0x8049 || VI_ATTR_MODEL_CODE==0x804A)}"
#define TL4000_FIND_PATTERN_ANY        "USB?*INSTR{VI_ATTR_MANF_ID==0x1313 && (VI_ATTR_MODEL_CODE==0x8040 || VI_ATTR_MODEL_CODE==0x8041 || VI_ATTR_MODEL_CODE==0x8042 || VI_ATTR_MODEL_CODE==0x8048 || VI_ATTR_MODEL_CODE==0x8049 || VI_ATTR_MODEL_CODE==0x804A || VI_ATTR_MODEL_CODE==0x8047 || VI_ATTR_MODEL_CODE==0x804F || VI_ATTR_MODEL_CODE==0x8046 || VI_ATTR_MODEL_CODE==0x804E)}"


/*---------------------------------------------------------------------------
 Buffers
---------------------------------------------------------------------------*/
#define TL4000_BUFFER_SIZE             (256)       // General buffer size
#define TL4000_ERR_DESCR_BUFFER_SIZE   (512)       // Buffer size for error messages

/*---------------------------------------------------------------------------
 Error/Warning Codes
   Note: The instrument returns errors within the range -512 .. +1023.
   The driver adds the value VI_INSTR_ERROR_OFFSET (0xBFFC0900). So the
   driver returns instrumetn errors in the range 0xBFFC0700 .. 0xBFFC0CFF.
---------------------------------------------------------------------------*/
// Offsets
#undef VI_INSTR_WARNING_OFFSET
#undef VI_INSTR_ERROR_OFFSET

#define VI_INSTR_WARNING_OFFSET        (0x3FFC0900L)
#define VI_INSTR_ERROR_OFFSET          (_VI_ERROR + 0x3FFC0900L)   //0xBFFC0900

// Driver warnings
#undef VI_INSTR_WARN_OVERFLOW
#undef VI_INSTR_WARN_UNDERRUN
#undef VI_INSTR_WARN_NAN
#undef VI_INSTR_WARN_LEGACY_FW

#define VI_INSTR_WARN_OVERFLOW         (VI_INSTR_WARNING_OFFSET + 0x01L)   //0x3FFC0901
#define VI_INSTR_WARN_UNDERRUN         (VI_INSTR_WARNING_OFFSET + 0x02L)   //0x3FFC0902
#define VI_INSTR_WARN_NAN              (VI_INSTR_WARNING_OFFSET + 0x03L)   //0x3FFC0903
#define VI_INSTR_WARN_LEGACY_FW        (VI_INSTR_WARNING_OFFSET + 0x10L)   //0x3FFC0910

/*---------------------------------------------------------------------------
 Value attributes
---------------------------------------------------------------------------*/
#define TL4000_ATTR_SET_VAL            (0)
#define TL4000_ATTR_MIN_VAL            (1)
#define TL4000_ATTR_MAX_VAL            (2)
#define TL4000_ATTR_DFLT_VAL           (3)

/*---------------------------------------------------------------------------
 Driver attributes
---------------------------------------------------------------------------*/
#define TL4000_ATTR_AUTO_ERROR_QUERY   (10)

/*---------------------------------------------------------------------------
 LD/PD polarity
---------------------------------------------------------------------------*/
#define TL4000_POL_CG                  (0)   // cathode grounded
#define TL4000_POL_AG                  (1)   // anode grounded

/*---------------------------------------------------------------------------
 Input terminals
---------------------------------------------------------------------------*/
#define TL4000_TERM_DSUB               (0)   // D-Sub connector
#define TL4000_TERM_BNC                (1)   // rear panel BNC connector

/*---------------------------------------------------------------------------
 Protection modes
---------------------------------------------------------------------------*/
#define TL4000_MODE_DISABLED           (0)   // Protection disabled
#define TL4000_MODE_OUTPUT_ENABLE      (1)   // act as LD output enable
#define TL4000_MODE_PROTECTION         (2)   // switch off LD output


/*===========================================================================

 GLOBAL USER-CALLABLE FUNCTION DECLARATIONS (Exportable Functions)

===========================================================================*/

/*===========================================================================

 Init/close

===========================================================================*/
/*---------------------------------------------------------------------------
 Initialize - This function initializes the instrument driver session and
 returns an instrument handle which is used in subsequent calls.
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_init (ViRsrc resourceName, ViBoolean IDQuery, ViBoolean resetDevice, ViPSession instrumentHandle);

/*---------------------------------------------------------------------------
 Close an instrument driver session
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_close (ViSession instrumentHandle);


/*===========================================================================

 Class: Configuration Functions.

===========================================================================*/

/*===========================================================================

 Subclass: Configuration Functions - Laser Driver

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get LD source operating mode
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdOperatingMode (ViSession instrumentHandle, ViInt16 laserDiodeOperatingMode);
ViStatus _VI_FUNC TL4000_getLdOperatingMode (ViSession instrumentHandle, ViPInt16 laserDiodeOperatingMode);

#define TL4000_LD_OPMODE_CC         (0)   // constant current mode
#define TL4000_LD_OPMODE_CP         (1)   // constant power mode

/*---------------------------------------------------------------------------
 Set/get LD source current limit
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdCurrLimit (ViSession instrumentHandle, ViReal64 laserDiodeCurrentLimit);
ViStatus _VI_FUNC TL4000_getLdCurrLimit (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 laserDiodeCurrentLimit);
ViStatus _VI_FUNC TL4000_isTrippedLdCurrLimit (ViSession instrumentHandle, ViPBoolean tripped);

/*---------------------------------------------------------------------------
 Set/get LD optical power limit
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdOptPowLimit (ViSession instrumentHandle, ViReal64 opticalPowerLimit);
ViStatus _VI_FUNC TL4000_getLdOptPowLimit (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 opticalPowerLimit);
ViStatus _VI_FUNC TL4000_isTrippedLdOptPowLimit (ViSession instrumentHandle, ViPBoolean tripped);

/*---------------------------------------------------------------------------
 Set/get photodiode current limit
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdCurrLimit (ViSession instrumentHandle, ViReal64 photodiodeCurrentLimit);
ViStatus _VI_FUNC TL4000_getPdCurrLimit (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 photodiodeCurrentLimit);

/*---------------------------------------------------------------------------
 Set/get thermopile voltage limit
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTpVoltLimit (ViSession instrumentHandle, ViReal64 thermopileVoltageLimit);
ViStatus _VI_FUNC TL4000_getTpVoltLimit (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 thermopileVoltageLimit);

/*===========================================================================

 Subclass: Configuration Functions - Laser Driver - Laser Diode Current

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get LD source operating current
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdCurrSetpoint (ViSession instrumentHandle, ViReal64 currentSetpoint);
ViStatus _VI_FUNC TL4000_getLdCurrSetpoint (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 laserDiodeCurrent);


/*===========================================================================

 Subclass: Configuration Functions- Laser Driver - Laser Diode Power

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get LD source operating power
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdOptPowSetpoint (ViSession instrumentHandle, ViReal64 opticalPowerSetpoint);
ViStatus _VI_FUNC TL4000_getLdOptPowSetpoint (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 opticalPowerSetpoint);

/*---------------------------------------------------------------------------
 Set/get LD source constant power mode feedback source
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPowerModeFbSrc (ViSession instrumentHandle, ViInt16 powerModeFeedbackSource);
ViStatus _VI_FUNC TL4000_getPowerModeFbSrc (ViSession instrumentHandle, ViPInt16 powerModeFeedbackSource);

#define TL4000_LDC_FEEDBACK_SRC_PD  (0)    // photodiode (current) feedback
#define TL4000_LDC_FEEDBACK_SRC_TP  (1)    // thermopile (voltage) feedback

/*---------------------------------------------------------------------------
 Set/get LD source constant power mode feedback speed setting
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPowerModeFbSpeed (ViSession instrumentHandle, ViReal64 speedSetting);
ViStatus _VI_FUNC TL4000_getPowerModeFbSpeed (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 speedSetting);

/*---------------------------------------------------------------------------
 Set/get photodiode current setpoint
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdCurrSetpoint (ViSession instrumentHandle, ViReal64 currentSetpoint);
ViStatus _VI_FUNC TL4000_getPdCurrSetpoint (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 currentSetpoint);

/*---------------------------------------------------------------------------
 Set/get thermopile voltage setpoint
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTpVoltSetpoint (ViSession instrumentHandle, ViReal64 voltageSetpoint);
ViStatus _VI_FUNC TL4000_getTpVoltSetpoint (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 voltageSetpoint);


/*===========================================================================

 Subclass: Configuration Functions - Laser Driver - QCW

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get QCW mode state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_switchQcwMode (ViSession instrumentHandle, ViBoolean QCWMode);
ViStatus _VI_FUNC TL4000_getQcwModeState (ViSession instrumentHandle, ViPBoolean QCWModeState);

/*---------------------------------------------------------------------------
 Set/get LD source constant current QCW mode pulse period
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setQCWPulsePeriod (ViSession instrumentHandle, ViReal64 pulsePeriod);
ViStatus _VI_FUNC TL4000_getQCWPulsePeriod (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 pulsePeriod);

/*---------------------------------------------------------------------------
 Set/get LD source constant current QCW mode pulse width
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setQCWPulseWidth (ViSession instrumentHandle, ViReal64 pulseWidth);
ViStatus _VI_FUNC TL4000_getQCWPulseWidth (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 pulseWidth);

/*---------------------------------------------------------------------------
 Set/get LD source constant current QCW mode duty cycle
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setQCWPulseDutyCycle (ViSession instrumentHandle, ViReal64 dutyCycle);
ViStatus _VI_FUNC TL4000_getQCWPulseDutyCycle (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 dutyCycle);

/*---------------------------------------------------------------------------
 Set/get LD source constant current QCW mode hold parameter
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setQCWHoldParam (ViSession instrumentHandle, ViInt16 holdParameter);
ViStatus _VI_FUNC TL4000_getQCWHoldParam (ViSession instrumentHandle, ViPInt16 holdParameter);

#define TL4000_PULSEHOLD_WIDTH      (0)   // pulse width held constant
#define TL4000_PULSEHOLD_DCYC       (1)   // duty cycle held constant

/*---------------------------------------------------------------------------
 Set/get LD source constant current QCW mode trigger source
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setQCWTrigSrc (ViSession instrumentHandle, ViInt16 triggerSource);
ViStatus _VI_FUNC TL4000_getQCWTrigSrc (ViSession instrumentHandle, ViPInt16 triggerSource);

#define TL4000_PULSE_TRIGSRC_INT    (0)   // internal trigger
#define TL4000_PULSE_TRIGSRC_EXT    (1)   // external trigger


/*===========================================================================

 Subclass: Configuration Functions - Laser Driver - Modulation

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get modulation state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_switchModulation (ViSession instrumentHandle, ViBoolean modulationState);
ViStatus _VI_FUNC TL4000_getModState (ViSession instrumentHandle, ViPBoolean modulationState);

/*---------------------------------------------------------------------------
 Set/get modulation source
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setModSource (ViSession instrumentHandle, ViInt16 source);
ViStatus _VI_FUNC TL4000_getModSource (ViSession instrumentHandle, ViPInt16 source);

#define TL4000_MODSRC_INT           (0)   // internal modulation
#define TL4000_MODSRC_EXT           (1)   // external BNC modulation input
#define TL4000_MODSRC_INT_EXT       (2)   // both modulation sources

/*---------------------------------------------------------------------------
 Set/get internal modulation shape
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setModShape (ViSession instrumentHandle, ViInt16 shape);
ViStatus _VI_FUNC TL4000_getModShape (ViSession instrumentHandle, ViPInt16 shape);

#define TL4000_MODSHAPE_SINE        (0)   // sinusoid
#define TL4000_MODSHAPE_TRIANGLE    (1)   // triangle
#define TL4000_MODSHAPE_SQUARE      (2)   // square

/*---------------------------------------------------------------------------
 Set/get internal modulation frequency [Hz]
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setModFreq (ViSession instrumentHandle, ViReal64 frequency);
ViStatus _VI_FUNC TL4000_getModFreq (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 frequency);

/*---------------------------------------------------------------------------
 Set/get internal modulation depth [%]
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setModDepth (ViSession instrumentHandle, ViReal64 depth);
ViStatus _VI_FUNC TL4000_getModDepth (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 depth);


/*===========================================================================

 Subclass: Configuration Functions - Laser Driver - Laser Diode output

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get laser diode polarity
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdPolarity (ViSession instrumentHandle, ViInt16 polarity);
ViStatus _VI_FUNC TL4000_getLdPolarity (ViSession instrumentHandle, ViPInt16 polarity);

/*---------------------------------------------------------------------------
 Set/get laser diode output switch-on delay
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdOutputDelay (ViSession instrumentHandle, ViReal64 delay);
ViStatus _VI_FUNC TL4000_getLdOutputDelay (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 delay);

/*---------------------------------------------------------------------------
 Set/get laser output protection voltage
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdOutputProtVolt (ViSession instrumentHandle, ViReal64 voltage);
ViStatus _VI_FUNC TL4000_getLdOutputProtVolt (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 voltage);

/*---------------------------------------------------------------------------
 Set/get rear panel LD-ENABLE input mode
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdEnableMode (ViSession instrumentHandle, ViInt16 mode);
ViStatus _VI_FUNC TL4000_getLdEnableMode (ViSession instrumentHandle, ViPInt16 mode);

/*---------------------------------------------------------------------------
 Set/get LD output low pass filter state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_switchLdOutpLpFltr (ViSession instrumentHandle, ViBoolean filterState);
ViStatus _VI_FUNC TL4000_getLdOutpLpFltrState (ViSession instrumentHandle, ViPBoolean filterState);

/*---------------------------------------------------------------------------
 Set/get LD output auto-on state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLdOutpAutoOnState (ViSession instrumentHandle, ViBoolean autoOnState);
ViStatus _VI_FUNC TL4000_getLdOutpAutoOnState (ViSession instrumentHandle, ViPBoolean autoOnState);


/*===========================================================================

 Subclass: Configuration Functions - Laser Driver - PD input

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get photodiode polarity
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdPolarity (ViSession instrumentHandle, ViInt16 polarity);
ViStatus _VI_FUNC TL4000_getPdPolarity (ViSession instrumentHandle, ViPInt16 polarity);

/*---------------------------------------------------------------------------
 Set/get photodiode range
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdRange (ViSession instrumentHandle, ViReal64 range);
ViStatus _VI_FUNC TL4000_getPdRange (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 range);

/*---------------------------------------------------------------------------
 Set/get photodiode BIAS state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_switchPdBIAS (ViSession instrumentHandle, ViBoolean BIASState);
ViStatus _VI_FUNC TL4000_getPdBIASState (ViSession instrumentHandle, ViPBoolean BIASState);

/*---------------------------------------------------------------------------
 Set/get photodiode BIAS voltage
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdBIASVolt (ViSession instrumentHandle, ViReal64 BIASVoltage);
ViStatus _VI_FUNC TL4000_getPdBIASVolt (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 BIASVoltage);

/*---------------------------------------------------------------------------
 Set/get photodiode input terminals
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdInpTerminals (ViSession instrumentHandle, ViInt16 terminals);
ViStatus _VI_FUNC TL4000_getPdInpTerminals (ViSession instrumentHandle, ViPInt16 terminals);

/*---------------------------------------------------------------------------
 Set/get photodiode responsivity
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPdResponsivity (ViSession instrumentHandle, ViReal64 response);
ViStatus _VI_FUNC TL4000_getPdResponsivity (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 responsivity);


/*===========================================================================

 Subclass: Configuration Functions - Laser Driver - Thermop./P Meter inp.

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get thermopile range
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTpRange (ViSession instrumentHandle, ViReal64 range);
ViStatus _VI_FUNC TL4000_getTpRange (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 range);

/*---------------------------------------------------------------------------
 Set/get thermopile input terminals
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTpInpTerminals (ViSession instrumentHandle, ViInt16 terminals);
ViStatus _VI_FUNC TL4000_getTpInpTerminals (ViSession instrumentHandle, ViPInt16 terminals);

/*---------------------------------------------------------------------------
 Set/get thermopile responsivity
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTpResponsivity (ViSession instrumentHandle, ViReal64 response);
ViStatus _VI_FUNC TL4000_getTpResponsivity (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 responsivity);


/*===========================================================================

 Subclass: Configuration Functions - TEC Driver

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get TEC source operating mode
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTecOperatingMode (ViSession instrumentHandle, ViInt16 operatingMode);
ViStatus _VI_FUNC TL4000_getTecOperatingMode (ViSession instrumentHandle, ViPInt16 operatingMode);

#define TL4000_TEC_OPMODE_CS     (0)   // Current source mode
#define TL4000_TEC_OPMODE_CT     (1)   // Constant Temperature mode

/*---------------------------------------------------------------------------
 Set/get TEC current limit
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTecCurrLimit (ViSession instrumentHandle, ViReal64 TECCurrentLimit);
ViStatus _VI_FUNC TL4000_getTecCurrLimit (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 TECCurrentLimit);


/*===========================================================================

 Subclass: Configuration Functions - TEC Driver - Temperature Sensor

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get sensor
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempSensor (ViSession instrumentHandle, ViInt16 sensor);
ViStatus _VI_FUNC TL4000_getTempSensor (ViSession instrumentHandle, ViPInt16 sensor);

#define  TL4000_SENS_AD590       (0)   // AD590 sensor
#define  TL4000_SENS_TH_LOW      (1)   // Thermistor (low range)
#define  TL4000_SENS_TH_HIGH     (2)   // Thermistor (high range)
#define  TL4000_SENS_PT100       (3)   // PT100 RTD sensor
#define  TL4000_SENS_PT1000      (4)   // PT1000 RTD sensor
#define  TL4000_SENS_LM35        (5)   // LM35 sensor
#define  TL4000_SENS_LM335       (6)   // LM335 sensor

/*---------------------------------------------------------------------------
 Set/get sensor temperature offset
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempSensOffset (ViSession instrumentHandle, ViReal64 temperatureOffset);
ViStatus _VI_FUNC TL4000_getTempSensOffset (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 temperatureOffset);


/*===========================================================================

 Subclass:  Config Func - TEC Driver - Temp Sensor - Thermistor

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get calculation method
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setThermistorMethod (ViSession instrumentHandle, ViInt16 method);
ViStatus _VI_FUNC TL4000_getThermistorMethod (ViSession instrumentHandle, ViPInt16 method);

#define TL4000_THMETH_EXP        (0)   // exponential method
#define TL4000_THMETH_SHH        (1)   // SHH method

/*---------------------------------------------------------------------------
 Set/get exponential parameters
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setThermistorExpParams (ViSession instrumentHandle, ViReal64 r0_value, ViReal64 t0_value, ViReal64 beta_value);
ViStatus _VI_FUNC TL4000_getThermistorExpParams (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 r0_Value, ViPReal64 t0_Value, ViPReal64 beta_Value);

/*---------------------------------------------------------------------------
 Set/get Steinhart-Hart parameters
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setThermistorShhParams (ViSession instrumentHandle, ViReal64 a_value, ViReal64 b_value, ViReal64 c_value);
ViStatus _VI_FUNC TL4000_getThermistorShhParams (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 a_Value, ViPReal64 b_Value, ViPReal64 c_Value);


/*===========================================================================

 Subclass: Config Func - TEC Driver - Constant Temperature Mode

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get temperature setpoint
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempSetpoint (ViSession instrumentHandle, ViReal64 temperatureSetpoint);
ViStatus _VI_FUNC TL4000_getTempSetpoint (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 temperatureSetpoint);

/*---------------------------------------------------------------------------
 Set/get setpoint temperature limits
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempSetpointLimits (ViSession instrumentHandle, ViReal64 lowLimit, ViReal64 highLimit);
ViStatus _VI_FUNC TL4000_getTempSetpointLimits (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 lowLimit, ViPReal64 highLimit);


/*===========================================================================

 Subclass: Config Func - TEC Driver - Constant Temp Mode - PID Control Loop

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get PID control parameters
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setPidParams (ViSession instrumentHandle, ViReal64 pShare, ViReal64 iShare, ViReal64 dShare, ViReal64 period);
ViStatus _VI_FUNC TL4000_getPidParams (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 pShare, ViPReal64 iShare, ViPReal64 dShare, ViPReal64 period);


/*===========================================================================

 Subclass: Config Func - TEC Driver - Current Source Mode

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get TEC current
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTecCurrSetpoint (ViSession instrumentHandle, ViReal64 TECCurrent);
ViStatus _VI_FUNC TL4000_getTecCurrSetpoint (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 TECCurrent);


/*===========================================================================

 Subclass: Config Func - TEC Driver - Temperature Protection

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get temperature window
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempProtWindow (ViSession instrumentHandle, ViReal64 val);
ViStatus _VI_FUNC TL4000_getTempProtWindow (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 pVal);

/*---------------------------------------------------------------------------
 Set/get temperature protection delay time
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempProtDelay (ViSession instrumentHandle, ViReal64 delay);
ViStatus _VI_FUNC TL4000_getTempProtDelay (ViSession instrumentHandle, ViInt16 attribute, ViPReal64 delay);

/*---------------------------------------------------------------------------
 Set/get temperature protection mode
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempProtMode (ViSession instrumentHandle, ViInt16 mode);
ViStatus _VI_FUNC TL4000_getTempProtMode (ViSession instrumentHandle, ViPInt16 mode);

/*---------------------------------------------------------------------------
 Set/get TEC output auto-on state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTecOutpAutoOnState (ViSession instrumentHandle, ViBoolean autoOnState);
ViStatus _VI_FUNC TL4000_getTecOutpAutoOnState (ViSession instrumentHandle, ViPBoolean autoOnState);

/*---------------------------------------------------------------------------
 Set/get TEC output voltage protection mode
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTecOutpVoltageProtMode (ViSession instrumentHandle, ViInt16 mode);
ViStatus _VI_FUNC TL4000_getTecOutpVoltageProtMode (ViSession instrumentHandle, ViPInt16 mode);


/*===========================================================================

 Subclass: Configuration Functions - System

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get display brightness
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setDispBrightness (ViSession instrumentHandle, ViReal64 val);
ViStatus _VI_FUNC TL4000_getDispBrightness (ViSession instrumentHandle, ViPReal64 pVal);

/*---------------------------------------------------------------------------
 Set/get display brightness auto-dimm state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setDispAutoDimmState (ViSession instrumentHandle, ViBoolean autoDimm);
ViStatus _VI_FUNC TL4000_getDispAutoDimmState (ViSession instrumentHandle, ViPBoolean autoDimm);

/*---------------------------------------------------------------------------
 Set/get display contrast
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setDispContrast (ViSession instrumentHandle, ViReal64 val);
ViStatus _VI_FUNC TL4000_getDispContrast (ViSession instrumentHandle, ViPReal64 pVal);

/*---------------------------------------------------------------------------
 Set/get line frequency
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setLineFreq (ViSession instrumentHandle, ViInt16 lineFrequency);
ViStatus _VI_FUNC TL4000_getLineFreq (ViSession instrumentHandle, ViPInt16 lineFrequency);

#define  TL4000_LFR_AUTO   (0) // Automatic detection
#define  TL4000_LFR_50HZ   (1) // 50Hz
#define  TL4000_LFR_60HZ   (2) // 60Hz

/*---------------------------------------------------------------------------
 Set/get temperature unit
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setTempUnit (ViSession instrumentHandle, ViInt16 temperatureUnit);
ViStatus _VI_FUNC TL4000_getTempUnit (ViSession instrumentHandle, ViPInt16 temperatureUnit);

#define  TL4000_U_CELS   (0) // Celsius
#define  TL4000_U_KELVIN (1) // Kelvin
#define  TL4000_U_FAHREN (2) // Fahrenheit


/*===========================================================================

 Subclass: Configuration Functions - System - Instrument Registers

===========================================================================*/
/*---------------------------------------------------------------------------
 Write/read register contents
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_writeRegister (ViSession instrumentHandle, ViInt16 reg, ViInt16 value);
ViStatus _VI_FUNC TL4000_readRegister (ViSession instrumentHandle, ViInt16 reg, ViPInt16 value);

#define TL4000_REG_STB                (0)   // Status Byte Register
#define TL4000_REG_SRE                (1)   // Service Request Enable
#define TL4000_REG_ESB                (2)   // Standard Event Status Register
#define TL4000_REG_ESE                (3)   // Standard Event Enable
#define TL4000_REG_OPER_COND          (4)   // Operation Condition Register
#define TL4000_REG_OPER_EVENT         (5)   // Operation Event Register
#define TL4000_REG_OPER_ENAB          (6)   // Operation Event Enable Register
#define TL4000_REG_OPER_PTR           (7)   // Operation Positive Transition Filter
#define TL4000_REG_OPER_NTR           (8)   // Operation Negative Transition Filter
#define TL4000_REG_QUES_COND          (9)   // Questionable Condition Register
#define TL4000_REG_QUES_EVENT         (10)  // Questionable Event Register
#define TL4000_REG_QUES_ENAB          (11)  // Questionable Event Enable Reg.
#define TL4000_REG_QUES_PTR           (12)  // Questionable Positive Transition Filter
#define TL4000_REG_QUES_NTR           (13)  // Questionable Negative Transition Filter
#define TL4000_REG_MEAS_COND          (14)  // Measurement Condition Register
#define TL4000_REG_MEAS_EVENT         (15)  // Measurement Event Register
#define TL4000_REG_MEAS_ENAB          (16)  // Measurement Event Enable Register
#define TL4000_REG_MEAS_PTR           (17)  // Measurement Positive Transition Filter
#define TL4000_REG_MEAS_NTR           (18)  // Measurement Negative Transition Filter
#define TL4000_REG_AUX_COND           (19)  // Auxiliary Condition Register
#define TL4000_REG_AUX_EVENT          (20)  // Auxiliary Event Register
#define TL4000_REG_AUX_ENAB           (21)  // Auxiliary Event Enable Register
#define TL4000_REG_AUX_PTR            (22)  // Auxiliary Positive Transition Filter
#define TL4000_REG_AUX_NTR            (23)  // Auxiliary Negative Transition Filter

// STATUS BYTE bit definitions (see IEEE488.2-1992 §11.2)
#define TL4000_STATBIT_STB_AUX        (0x01)   // Auxiliary summary
#define TL4000_STATBIT_STB_MEAS       (0x02)   // Device Measurement Summary
#define TL4000_STATBIT_STB_EAV        (0x04)   // Error available
#define TL4000_STATBIT_STB_QUES       (0x08)   // Questionable Status Summary
#define TL4000_STATBIT_STB_MAV        (0x10)   // Message available
#define TL4000_STATBIT_STB_ESB        (0x20)   // Event Status Bit
#define TL4000_STATBIT_STB_MSS        (0x40)   // Master summary status
#define TL4000_STATBIT_STB_OPER       (0x80)   // Operation Status Summary

// STANDARD EVENT STATUS REGISTER bit definitions (see IEEE488.2-1992 §11.5.1)
#define TL4000_STATBIT_ESR_OPC        (0x01)   // Operation complete
#define TL4000_STATBIT_ESR_RQC        (0x02)   // Request control
#define TL4000_STATBIT_ESR_QYE        (0x04)   // Query error
#define TL4000_STATBIT_ESR_DDE        (0x08)   // Device-Specific error
#define TL4000_STATBIT_ESR_EXE        (0x10)   // Execution error
#define TL4000_STATBIT_ESR_CME        (0x20)   // Command error
#define TL4000_STATBIT_ESR_URQ        (0x40)   // User request
#define TL4000_STATBIT_ESR_PON        (0x80)   // Power on

// QUESTIONABLE STATUS REGISTER bit definitions (see SCPI 99.0 §9)
#define TL4000_STATBIT_QUES_VOLT      (0x0001) // questionable voltage measurement
#define TL4000_STATBIT_QUES_CURR      (0x0002) // questionable current measurement
#define TL4000_STATBIT_QUES_TIME      (0x0004) // questionable time measurement
#define TL4000_STATBIT_QUES_POW       (0x0008) // questionable power measurement
#define TL4000_STATBIT_QUES_TEMP      (0x0010) // questionable temperature measurement
#define TL4000_STATBIT_QUES_FREQ      (0x0020) // questionable frequency measurement
#define TL4000_STATBIT_QUES_PHAS      (0x0040) // questionable phase measurement
#define TL4000_STATBIT_QUES_MOD       (0x0080) // questionable modulation measurement
#define TL4000_STATBIT_QUES_CAL       (0x0100) // questionable calibration
#define TL4000_STATBIT_QUES_9         (0x0200) // reserved
#define TL4000_STATBIT_QUES_10        (0x0400) // reserved
#define TL4000_STATBIT_QUES_11        (0x0800) // reserved
#define TL4000_STATBIT_QUES_12        (0x1000) // reserved
#define TL4000_STATBIT_QUES_INST      (0x2000) // instrument summary
#define TL4000_STATBIT_QUES_WARN      (0x4000) // command warning
#define TL4000_STATBIT_QUES_15        (0x8000) // reserved

// OPERATION STATUS REGISTER bit definitions (see SCPI 99.0 §9)
#define TL4000_STATBIT_OPER_CAL       (0x0001) // The instrument is currently performing a calibration.
#define TL4000_STATBIT_OPER_SETT      (0x0002) // The instrument is waiting for signals it controls to stabilize enough to begin measurements.
#define TL4000_STATBIT_OPER_RANG      (0x0004) // The instrument is currently changing its range.
#define TL4000_STATBIT_OPER_SWE       (0x0008) // A sweep is in progress.
#define TL4000_STATBIT_OPER_MEAS      (0x0010) // The instrument is actively measuring.
#define TL4000_STATBIT_OPER_TRIG      (0x0020) // The instrument is in a “wait for trigger” state of the trigger model.
#define TL4000_STATBIT_OPER_ARM       (0x0040) // The instrument is in a “wait for arm” state of the trigger model.
#define TL4000_STATBIT_OPER_CORR      (0x0080) // The instrument is currently performing a correction (PID auto-tune).
#define TL4000_STATBIT_OPER_8         (0x0100) // reserved
#define TL4000_STATBIT_OPER_9         (0x0200) // reserved
#define TL4000_STATBIT_OPER_10        (0x0400) // reserved
#define TL4000_STATBIT_OPER_LD_ON     (0x0800) // The LD output is switched ON
#define TL4000_STATBIT_OPER_TEC_ON    (0x1000) // The TEC output is switched ON
#define TL4000_STATBIT_OPER_INST      (0x2000) // One of n multiple logical instruments is reporting OPERational status.
#define TL4000_STATBIT_OPER_PROG      (0x4000) // A user-defined programming is currently in the run state.
#define TL4000_STATBIT_OPER_15        (0x8000) // reserved

// Thorlabs defined MEASRUEMENT STATUS REGISTER bit definitions
#define TL4000_STATBIT_MEAS_KEYLOCK   (0x0001) // Keylock protection active (ITC4000, LDC4000, CLD1000)
#define TL4000_STATBIT_MEAS_LD_OPEN   (0x0002) // LD output compliance voltage limit reached (ITC4000, LDC4000, CLD1000)
#define TL4000_STATBIT_MEAS_INTERLOCK (0x0004) // Interlock circuit active (ITC4000, LDC4000, CLD1000)
#define TL4000_STATBIT_MEAS_I_LIMIT   (0x0008) // LD output current limit reached (ITC4000, LDC4000, CLD1000)
#define TL4000_STATBIT_MEAS_INHIBIT   (0x0010) // LD output inhibit circuit active (ITC4000, LDC4000)
#define TL4000_STATBIT_MEAS_P_LIMIT   (0x0020) // LD power limit reached (ITC4000, LDC4000)
#define TL4000_STATBIT_MEAS_6         (0x0040) // reserved
#define TL4000_STATBIT_MEAS_TEC_ILIM  (0x0080) // TEC output current limit reached (CLD1000)
#define TL4000_STATBIT_MEAS_WIN_PROT  (0x0100) // Temperature window protection is active (ITC4000, TED4000, CLD1000)
#define TL4000_STATBIT_MEAS_WIN_FAIL  (0x0200) // Temperature window failure (the temperature is out of the specified window) (ITC4000, TED4000, CLD1000)
#define TL4000_STATBIT_MEAS_NO_SENSOR (0x0400) // The selected temperature sensor is not propperly connected (ITC4000, TED4000, CLD1000)
#define TL4000_STATBIT_MEAS_TEC_OPEN  (0x0800) // TEC output compliance voltage reached (ITC4000, TED4000, CLD1000)
#define TL4000_STATBIT_MEAS_NO_CABLE  (0x1000) // TEC cable connection failure (ITC4000, TED4000)
#define TL4000_STATBIT_MEAS_MOUNT     (0x2000) // Mount/socket failure (CLD1000)
#define TL4000_STATBIT_MEAS_OVERTEMP  (0x4000) // The instrument's temperature is too high
#define TL4000_STATBIT_MEAS_15        (0x8000) // reserved

// Thorlabs defined AUXILIARY STATUS REGISTER bit definitions
#define TL4000_STATBIT_AUX_GPIO1      (0x0001) // GPIO port #1 (ITC4000, LDC4000, TED4000)
#define TL4000_STATBIT_AUX_GPIO2      (0x0002) // GPIO port #2 (ITC4000, LDC4000, TED4000)
#define TL4000_STATBIT_AUX_GPIO3      (0x0004) // GPIO port #3 (ITC4000, LDC4000, TED4000)
#define TL4000_STATBIT_AUX_GPIO4      (0x0008) // GPIO port #4 (ITC4000, LDC4000, TED4000)
#define TL4000_STATBIT_AUX_4          (0x0010) // reserved
#define TL4000_STATBIT_AUX_5          (0x0020) // reserved
#define TL4000_STATBIT_AUX_6          (0x0040) // reserved
#define TL4000_STATBIT_AUX_7          (0x0080) // reserved
#define TL4000_STATBIT_AUX_8          (0x0100) // reserved
#define TL4000_STATBIT_AUX_9          (0x0200) // reserved
#define TL4000_STATBIT_AUX_10         (0x0400) // reserved
#define TL4000_STATBIT_AUX_11         (0x0800) // reserved
#define TL4000_STATBIT_AUX_12         (0x1000) // reserved
#define TL4000_STATBIT_AUX_13         (0x2000) // reserved
#define TL4000_STATBIT_AUX_14         (0x4000) // reserved
#define TL4000_STATBIT_AUX_15         (0x8000) // reserved


/*===========================================================================

 Class: Action/Status Functions

===========================================================================*/
/*===========================================================================

 Subclass: Action/Status Functions - Laser Driver

===========================================================================*/
/*---------------------------------------------------------------------------
 Switch LD output/get state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_switchLdOutput (ViSession instrumentHandle, ViBoolean output);
ViStatus _VI_FUNC TL4000_getLdOutputState (ViSession instrumentHandle, ViPBoolean output);

/*---------------------------------------------------------------------------
 Get LD output protection states
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_isTrippedLdOutputProtInterlock (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedLdOutputProtEnable (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedLdOutputProtKeylock (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedLdOutputProtTemperature (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedLdOutputProtOvertemp (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedLdOutputProtVolt (ViSession instrumentHandle, ViPBoolean tripped);


/*===========================================================================

 Subclass: Action/Status Functions - TEC Driver

===========================================================================*/
/*---------------------------------------------------------------------------
 Switch TEC output/get state
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_switchTecOutput (ViSession instrumentHandle, ViBoolean output);
ViStatus _VI_FUNC TL4000_getTecOutputState (ViSession instrumentHandle, ViPBoolean output);

/*---------------------------------------------------------------------------
 Get TEC output protection states
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_isTrippedTecOutputProtCable (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedTecOutputProtTransducer (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedTecOutputProtOvertemp (ViSession instrumentHandle, ViPBoolean tripped);
ViStatus _VI_FUNC TL4000_isTrippedTecOutputProtVoltage (ViSession instrumentHandle, ViPBoolean tripped);


/*===========================================================================

 Subclass: Action/Status Functions - TEC Driver - PID Auto-Tune

===========================================================================*/
/*---------------------------------------------------------------------------
 Start/Cancel Auto-PID procedure
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_startAutoPid (ViSession instrumentHandle);
ViStatus _VI_FUNC TL4000_cancelAutoPid (ViSession instrumentHandle);
ViStatus _VI_FUNC TL4000_getAutoPidState (ViSession instrumentHandle, ViPInt16 state, ViPInt16 phase, ViPInt16 count);

/*---------------------------------------------------------------------------
 Get/Transfer Auto-PID parameters
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_getAutoPidParams (ViSession instrumentHandle, ViPReal64 pShare, ViPReal64 iShare, ViPReal64 dShare, ViPReal64 period);
ViStatus _VI_FUNC TL4000_transferAutoPidParams (ViSession instrumentHandle);


/*===========================================================================

 Class: Data Functions.

===========================================================================*/
/*---------------------------------------------------------------------------
 Measure - Laser Driver
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_measLdCurr (ViSession instrumentHandle, ViPReal64 current);
ViStatus _VI_FUNC TL4000_measLdVolt (ViSession instrumentHandle, ViPReal64 voltage);
ViStatus _VI_FUNC TL4000_measLdPow (ViSession instrumentHandle, ViPReal64 power);
ViStatus _VI_FUNC TL4000_measPdCurr (ViSession instrumentHandle, ViPReal64 current);
ViStatus _VI_FUNC TL4000_measTpVolt (ViSession instrumentHandle, ViPReal64 voltage);
ViStatus _VI_FUNC TL4000_measLdOptPowPd (ViSession instrumentHandle, ViPReal64 power);
ViStatus _VI_FUNC TL4000_measLdOptPowTp (ViSession instrumentHandle, ViPReal64 power);
ViStatus _VI_FUNC TL4000_measLdMultiple (ViSession instrumentHandle, ViPReal64 laserCurrent, ViPReal64 laserVoltage, ViPReal64 power, ViPReal64 photodiodeCurrent, ViPReal64 opticalPowerPd, ViPReal64 thermopileVoltage, ViPReal64 opticalPowerTh);

/*---------------------------------------------------------------------------
 Measure - TEC Driver
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_measTemp (ViSession instrumentHandle, ViPReal64 temperature);
ViStatus _VI_FUNC TL4000_measTempSensor (ViSession instrumentHandle, ViPReal64 sensorSignal);
ViStatus _VI_FUNC TL4000_measTecCurr (ViSession instrumentHandle, ViPReal64 TECCurrent);
ViStatus _VI_FUNC TL4000_measTecVolt (ViSession instrumentHandle, ViPReal64 TECVoltage);
ViStatus _VI_FUNC TL4000_measTecPow (ViSession instrumentHandle, ViPReal64 TECPower);


/*===========================================================================

 Subsubclass: Data Functions - Low Level Measurements

===========================================================================*/
/*---------------------------------------------------------------------------
 Abort measurement
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_abort (ViSession instrumentHandle);

/*---------------------------------------------------------------------------
 Configure measurement
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_configure (ViSession instrumentHandle, ViInt16 parameter);

// parameters
#define TL4000_MEAS_TEC_TEMP           0        // TEC temperature
#define TL4000_MEAS_TEC_CURR           1        // TEC current
#define TL4000_MEAS_TEC_VOLT           2        // TEC voltage
#define TL4000_MEAS_TEC_POW            3        // TEC electrical power
#define TL4000_MEAS_TEC_SENS           4        // TEC sensor
#define TL4000_MEAS_LD_CURR            10       // Laser Diode current
#define TL4000_MEAS_LD_VOLT            11       // Laser Diode voltage
#define TL4000_MEAS_LD_POW             12       // Laser Diode electrical power
#define TL4000_MEAS_PD_CURR            13       // Photodiode current
#define TL4000_MEAS_OPT_POW_PD         14       // optical power via photodiode
#define TL4000_MEAS_TP_VOLT            15       // Thermopile/Power Meter voltage
#define TL4000_MEAS_OPT_POW_TP         16       // optical power via thermopile/power meter input

/*---------------------------------------------------------------------------
 Initiate measurement
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_initiate (ViSession instrumentHandle);

/*---------------------------------------------------------------------------
 Fetch measurement values - Laser Driver
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_fetchLdCurr (ViSession instrumentHandle, ViPReal64 current);
ViStatus _VI_FUNC TL4000_fetchLdVolt (ViSession instrumentHandle, ViPReal64 voltage);
ViStatus _VI_FUNC TL4000_fetchLdPow (ViSession instrumentHandle, ViPReal64 power);
ViStatus _VI_FUNC TL4000_fetchPdCurr (ViSession instrumentHandle, ViPReal64 current);
ViStatus _VI_FUNC TL4000_fetchTpVolt (ViSession instrumentHandle, ViPReal64 voltage);
ViStatus _VI_FUNC TL4000_fetchLdOptPowPd (ViSession instrumentHandle, ViPReal64 power);
ViStatus _VI_FUNC TL4000_fetchLdOptPowTp (ViSession instrumentHandle, ViPReal64 power);
ViStatus _VI_FUNC TL4000_fetchLdMultiple (ViSession instrumentHandle, ViPReal64 laserCurrent, ViPReal64 laserVoltage, ViPReal64 power, ViPReal64 photodiodeCurrent, ViPReal64 opticalPowerPd, ViPReal64 thermopileVoltage, ViPReal64 opticalPowerTh);

/*---------------------------------------------------------------------------
 Fetch measurement values - TEC Driver
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_fetchTemp (ViSession instrumentHandle, ViPReal64 temperature);
ViStatus _VI_FUNC TL4000_fetchTempSensor (ViSession instrumentHandle, ViPReal64 sensorSignal);
ViStatus _VI_FUNC TL4000_fetchTecCurr (ViSession instrumentHandle, ViPReal64 TECCurrent);
ViStatus _VI_FUNC TL4000_fetchTecVolt (ViSession instrumentHandle, ViPReal64 TECVoltage);
ViStatus _VI_FUNC TL4000_fetchTecPow (ViSession instrumentHandle, ViPReal64 TECPower);


/*===========================================================================

 Class: Utility Functions.

==========================================================================*/
/*---------------------------------------------------------------------------
 Set/get driver attribute.
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setAttribute (ViSession instrumentHandle, ViAttr attribute, ViUInt32 value);
ViStatus _VI_FUNC TL4000_getAttribute (ViSession instrumentHandle, ViAttr attribute, ViPUInt32 value);

/*---------------------------------------------------------------------------
 Identification query
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_identificationQuery (ViSession instrumentHandle, ViChar _VI_FAR manufacturerName[], ViChar _VI_FAR deviceName[], ViChar _VI_FAR serialNumber[], ViChar _VI_FAR firmwareRevision[]);

/*---------------------------------------------------------------------------
 Get calibration message
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_calibrationMessage (ViSession instrumentHandle, ViChar _VI_FAR message[]);

/*---------------------------------------------------------------------------
 Reset the instrument.
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_reset (ViSession instrumentHandle);

/*---------------------------------------------------------------------------
 Beep
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_beep (ViSession instrumentHandle);

/*---------------------------------------------------------------------------
 Run Self-Test routine.
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_selfTest (ViSession instrumentHandle, ViPInt16 selfTestResult, ViChar _VI_FAR selfTestMessage[]);

/*---------------------------------------------------------------------------
 Error Query
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_errorQuery (ViSession instrumentHandle, ViPInt32 errorNumber, ViChar _VI_FAR errorMessage[]);

/*---------------------------------------------------------------------------
 Get error description.
 This function translates the error return value from the instrument driver
 into a user-readable string.
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_errorMessage (ViSession instrumentHandle, ViStatus statusCode, ViChar _VI_FAR description[]);

/*---------------------------------------------------------------------------
 Revision Query
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_revisionQuery (ViSession instrumentHandle, ViChar _VI_FAR instrumentDriverRevision[], ViChar _VI_FAR firmwareRevision[]);


/*===========================================================================

 SubClass: Utility Functions - Setup Memory

===========================================================================*/
/*---------------------------------------------------------------------------
 Save/Recall setup
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_saveSetup (ViSession instrumentHandle, ViInt16 setup);
ViStatus _VI_FUNC TL4000_recallSetup (ViSession instrumentHandle, ViInt16 setup);

/*---------------------------------------------------------------------------
 Get/Set setup label
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setSetupLabel (ViSession instrumentHandle, ViInt16 setup, ViChar _VI_FAR label[]);
ViStatus _VI_FUNC TL4000_getSetupLabel (ViSession instrumentHandle, ViInt16 setup, ViChar _VI_FAR label[]);

/*---------------------------------------------------------------------------
 Get number of setups available
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_getNumSetup (ViSession instrumentHandle, ViPInt16 number_ofSetups);


/*===========================================================================

 Subclass: Utility Functions - Digital I/O

===========================================================================*/
/*---------------------------------------------------------------------------
 Set/get digital I/O direction
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setDigIoDirection (ViSession instrumentHandle, ViBoolean IO1, ViBoolean IO2, ViBoolean IO3, ViBoolean IO4);
ViStatus _VI_FUNC TL4000_getDigIoDirection (ViSession instrumentHandle, ViPBoolean IO1, ViPBoolean IO2, ViPBoolean IO3, ViPBoolean IO4);

#define TL4000_IODIR_IN    (VI_OFF)
#define TL4000_IODIR_OUT   (VI_ON)

/*---------------------------------------------------------------------------
 Set/get digital I/O output data
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_setDigIoOutput (ViSession instrumentHandle, ViBoolean IO1, ViBoolean IO2, ViBoolean IO3, ViBoolean IO4);
ViStatus _VI_FUNC TL4000_getDigIoOutput (ViSession instrumentHandle, ViPBoolean IO1, ViPBoolean IO2, ViPBoolean IO3, ViPBoolean IO4);

#define TL4000_IOLVL_LOW   (VI_OFF)
#define TL4000_IOLVL_HIGH  (VI_ON)

/*---------------------------------------------------------------------------
 Get digital I/O port data
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_getDigIoPort (ViSession instrumentHandle, ViPBoolean IO1, ViPBoolean IO2, ViPBoolean IO3, ViPBoolean IO4);


/*===========================================================================

 Subclass: Utility Functions - Raw I/O

===========================================================================*/
/*---------------------------------------------------------------------------
 Write to Instrument
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_writeRaw (ViSession instrumentHandle, ViString command);

/*---------------------------------------------------------------------------
 Read from Instrument
---------------------------------------------------------------------------*/
ViStatus _VI_FUNC TL4000_readRaw (ViSession instrumentHandle, ViChar _VI_FAR buffer[], ViUInt32 size, ViPUInt32 returnCount);




#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif   /* _TL4000_DRIVER_HEADER_ */

/****************************************************************************

  End of Header file

****************************************************************************/
