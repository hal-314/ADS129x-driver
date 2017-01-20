/*
 * This file define register and commands constant for ADS129xx chip family
 * 
 * Register constant that start with B_* is a mask to modify a certain bit in the register.
 * Therefore, when they are used to write a value in a register,
 * it is always necessary to add the right value for 
 * reserved bits (constant RESERVED_BITS in the namespace) in the register.
 * So, before write in the register, the RESERVED_BITS mask MUST BE added if B_* are 
 * the only constant used in order to write the correct values in the reserved bits.
 * 
 * Example: enable high-power mode for ADS12xx -> write 1 in the field HR in the register config1:
 *    byte valueToWrite = ads::registers::config1::B_HR | ads::registers::id::RESERVED_BITS
 *    
 * The other constants without underscored are intended to use to create the right value to write
 * in the register by joining with OR operations. The right value for the reserved bits 
 * in the register are included in these constants. If you want to change a single bit in 
 * the register, use B_* constant. 
 * 
 * Register constant that start with a underscore, ex: _BASE_REG_ADDR, are private constants.
 */

/* =========== ADS129XX Command and register constants ================== */
#ifndef ADS129XX_CONSTANTS_H_
#define ADS129XX_CONSTANTS_H_

#include <Arduino.h>

namespace ads {

namespace commands {
// system commands
const byte WAKEUP = 0x02;
const byte STANDBY = 0x04;
const byte RESET = 0x06;
const byte START = 0x08;
const byte STOP = 0x0a;

// read commands
const byte RDATAC = 0x10;
const byte SDATAC = 0x11;
const byte RDATA = 0x12;

// register commands
const byte RREG = 0x20;
const byte WREG = 0x40;
}

namespace registers {

// Read-only registers
namespace id {
const byte REG_ADDR = 0x00;
const byte RESERVED_BITS = 0x10;
const boolean READ_ONLY_REGISTER = true;

const byte B_DEV_ID7 = 0x80;
const byte B_DEV_ID6 = 0x40;
const byte B_DEV_ID5 = 0x20;
const byte B_DEV_ID2 = 0x04;
const byte B_DEV_ID1 = 0x02;
const byte B_DEV_ID0 = 0x01;

const byte _ID_ADS129x = B_DEV_ID7;
const byte _ID_ADS129xR = (B_DEV_ID7 | B_DEV_ID6);

const byte _ID_4CHAN = 0x00;
const byte _ID_6CHAN = B_DEV_ID0;
const byte _ID_8CHAN = B_DEV_ID1;

const byte ID_ADS1294 = (_ID_ADS129x | _ID_4CHAN | RESERVED_BITS);
const byte ID_ADS1296 = (_ID_ADS129x | _ID_6CHAN | RESERVED_BITS);
const byte ID_ADS1298 = (_ID_ADS129x | _ID_8CHAN | RESERVED_BITS);
const byte ID_ADS1294R = (_ID_ADS129xR | _ID_4CHAN | RESERVED_BITS);
const byte ID_ADS1296R = (_ID_ADS129xR | _ID_6CHAN | RESERVED_BITS);
const byte ID_ADS1298R = (_ID_ADS129xR | _ID_8CHAN | RESERVED_BITS);
}

namespace config1 {
const byte REG_ADDR = 0X01;
const byte RESERVED_BITS  = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x06;

// Constants helpers
const byte B_HR = 0x80;
const byte B_DR2 = 0x04;
const byte B_DR1 = 0x02;
const byte B_DR0 = 0x01;

const byte B_DAISY_EN = 0x40;
const byte B_CLK_EN = 0x20;

// FIXME: Datasheet says in 32 kSPS and 64 kSPS, the bits per channel send by ADS is 16 BUT ADS max sample frequency is 32 kSPS. Check it (page 53, Readback length) when new version of the datasheet is available.
// High resolution mode
#if ADS_BITS_PER_CHANNEL == 16
const byte HIGH_RES_32k_SPS = B_HR | RESERVED_BITS;
#elif ADS_BITS_PER_CHANNEL == 24
const byte HIGH_RES_16k_SPS = (B_HR | B_DR0 | RESERVED_BITS);
const byte HIGH_RES_8k_SPS = (B_HR | B_DR1 | RESERVED_BITS);
const byte HIGH_RES_4k_SPS = (B_HR | B_DR1 | B_DR0 | RESERVED_BITS);
const byte HIGH_RES_2k_SPS = (B_HR | B_DR2 | RESERVED_BITS);
const byte HIGH_RES_1k_SPS = (B_HR | B_DR2 | B_DR0 | RESERVED_BITS);
const byte HIGH_RES_500_SPS = (B_HR | B_DR2 | B_DR1 | RESERVED_BITS);

// Low resolution mode
const byte LOW_POWR_16k_SPS = 0x00 | RESERVED_BITS;
const byte LOW_POWR_8k_SPS = B_DR0 | RESERVED_BITS;
const byte LOW_POWR_4k_SPS = B_DR1 | RESERVED_BITS;
const byte LOW_POWR_2k_SPS = (B_DR1 | B_DR0 | RESERVED_BITS);
const byte LOW_POWR_1k_SPS = B_DR2 | RESERVED_BITS;
const byte LOW_POWR_500_SPS = (B_DR2 | B_DR0 | RESERVED_BITS);
const byte LOW_POWR_250_SPS = (B_DR2 | B_DR1 | RESERVED_BITS);
#endif
}

namespace config2 {
const byte REG_ADDR = 0x02;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
// FIXME: Datasheet says that RESET_VALUE is 0x40 BUT also says that in bit 7 and 8, zero must be written. I check the value and it is zero for ADS1294. So I put here 0x00. Check it (page 68, Config2: configuration ...) when new version of the datasheet is available.
const byte RESET_VALUE = 0x00; 

const byte B_WCT_CHOP = 0x20;
const byte B_INT_TEST = 0x10;
const byte B_TEST_AMP = 0x04;

const byte TEST_SOURCE_EXTERNAL = RESERVED_BITS;
const byte TEST_SOURCE_INTERNAL = B_INT_TEST | RESERVED_BITS;

const byte TEST_FREQ_2HZ = (B_INT_TEST | 0x00 | RESERVED_BITS);
const byte TEST_FREQ_4HZ = (B_INT_TEST | 0x01 | RESERVED_BITS);
const byte TEST_FREQ_DC = (B_INT_TEST | 0x03 | RESERVED_BITS);
}

namespace config3 {
const byte REG_ADDR = 0X03;
const byte RESERVED_BITS = 0x40;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x40;

// Remember to wait 150 microseconds if internal reference will be used. See page 15, section Electrical Characteristicsm - Internal Reference, in the datasheet
const byte B_PD_REFBUF = 0x80;
const byte B_VREF_4V = 0x20;
const byte B_RLD_MEAS = 0x10;
const byte B_RLDREF_INT = 0x08;
const byte B_PD_RLD = 0x04;
const byte B_RLD_LOFF_SENS = 0x02;
const byte B_RLD_STAT = 0x01;
}

namespace loff {
const byte REG_ADDR = 0X04;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

const byte B_COMP_TH2 = 0x80;
const byte B_COMP_TH1 = 0x40;
const byte B_COMP_TH0 = 0x20;
const byte B_VLEAD_OFF_EN = 0x10;
const byte B_ILEAD_OFF1 = 0x08;
const byte B_ILEAD_OFF0 = 0x04;
const byte B_FLEAD_OFF1 = 0x02;
const byte B_FLEAD_OFF0 = 0x01;

const byte COMP_TH_95 = 0x00 | RESERVED_BITS;
const byte COMP_TH_92_5 = B_COMP_TH0 | RESERVED_BITS;
const byte COMP_TH_90 = B_COMP_TH1 | RESERVED_BITS;
const byte COMP_TH_87_5 = (B_COMP_TH1 | B_COMP_TH0 | RESERVED_BITS);
const byte COMP_TH_85 = B_COMP_TH2 | RESERVED_BITS;
const byte COMP_TH_80 = (B_COMP_TH2 | B_COMP_TH0 | RESERVED_BITS);
const byte COMP_TH_75 = (B_COMP_TH2 | B_COMP_TH1 | RESERVED_BITS);
const byte COMP_TH_70 = (B_COMP_TH2 | B_COMP_TH1 | B_COMP_TH0 | RESERVED_BITS);

const byte ILEAD_OFF_6nA = 0x00 | RESERVED_BITS;
const byte ILEAD_OFF_12nA = B_ILEAD_OFF0 | RESERVED_BITS;
const byte ILEAD_OFF_18nA = B_ILEAD_OFF1 | RESERVED_BITS;
const byte ILEAD_OFF_24nA = (B_ILEAD_OFF1 | B_ILEAD_OFF0 | RESERVED_BITS);

const byte FLEAD_OFF_AC = B_FLEAD_OFF0 | RESERVED_BITS;
const byte FLEAD_OFF_DC = (B_FLEAD_OFF1 | B_FLEAD_OFF0 | RESERVED_BITS);
}

namespace chnSet {  
const byte _BASE_REG_ADDR = 0x04; // Base register address
const byte REG_ADDR_CH1SET = _BASE_REG_ADDR + 1;
const byte REG_ADDR_CH2SET = _BASE_REG_ADDR + 2;
const byte REG_ADDR_CH3SET = _BASE_REG_ADDR + 3;
const byte REG_ADDR_CH4SET = _BASE_REG_ADDR + 4;

#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte REG_ADDR_CH5SET = _BASE_REG_ADDR + 5;
const byte REG_ADDR_CH6SET = _BASE_REG_ADDR + 6;
#endif
#if ADS_N_CHANNELS > 6 // ADS1298 or ADS1298R
const byte REG_ADDR_CH7SET = _BASE_REG_ADDR + 7;
const byte REG_ADDR_CH8SET = _BASE_REG_ADDR + 8;
#endif

const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

const byte B_PDn = 0x80;
const byte B_GAINn2 = 0x40;
const byte B_GAINn1 = 0x20;
const byte B_GAINn0 = 0x10;
const byte B_MUXn2 = 0x04;
const byte B_MUXn1 = 0x02;
const byte B_MUXn0 = 0x01;

const byte ENABLE_CHANNEL = RESERVED_BITS;
const byte DISABLE_CHANNEL = B_PDn | RESERVED_BITS;

const byte GAIN_1X = B_GAINn0 | RESERVED_BITS;
const byte GAIN_2X = B_GAINn1 | RESERVED_BITS;
const byte GAIN_3X = (B_GAINn1 | B_GAINn0 | RESERVED_BITS);
const byte GAIN_4X = B_GAINn2 | RESERVED_BITS;
const byte GAIN_6X = 0x00 | RESERVED_BITS;
const byte GAIN_8X = (B_GAINn2 | B_GAINn0 | RESERVED_BITS);
const byte GAIN_12X = (B_GAINn2 | B_GAINn1 | RESERVED_BITS);

const byte ELECTRODE_INPUT = 0x00 | RESERVED_BITS;
const byte SHORTED = B_MUXn0 | RESERVED_BITS;
const byte RLD_INPUT = B_MUXn1 | RESERVED_BITS;
const byte MVDD = (B_MUXn1 | B_MUXn0 | RESERVED_BITS);
const byte TEMP = B_MUXn2 | RESERVED_BITS;
const byte TEST_SIGNAL = (B_MUXn2 | B_MUXn0 | RESERVED_BITS);
const byte RLD_DRP = (B_MUXn2 | B_MUXn1 | RESERVED_BITS);
const byte RLD_DRN = (B_MUXn2 | B_MUXn1 | B_MUXn0 | RESERVED_BITS);
}

namespace rldSensp {
const byte REG_ADDR = 0X0D;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_N_CHANNELS > 6 // ADS1298 or ADS1298R
const byte B_RLD8P = 0x80 | RESERVED_BITS;
const byte B_RLD7P = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_RLD6P = 0x20 | RESERVED_BITS;
const byte B_RLD5P = 0x10 | RESERVED_BITS;
#endif
const byte B_RLD4P = 0x08 | RESERVED_BITS;
const byte B_RLD3P = 0x04 | RESERVED_BITS;
const byte B_RLD2P = 0x02 | RESERVED_BITS;
const byte B_RLD1P = 0x01 | RESERVED_BITS;
}

namespace rldSensn {
const byte REG_ADDR = 0X0E;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_N_CHANNELS > 6 // ADS1298 or ADS1298R
const byte B_RLD8N = 0x80 | RESERVED_BITS;
const byte B_RLD7N = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_RLD6N = 0x20 | RESERVED_BITS;
const byte B_RLD5N = 0x10 | RESERVED_BITS;
#endif
const byte B_RLD4N = 0x08 | RESERVED_BITS;
const byte B_RLD3N = 0x04 | RESERVED_BITS;
const byte B_RLD2N = 0x02 | RESERVED_BITS;
const byte B_RLD1N = 0x01 | RESERVED_BITS;
}

namespace loffSensp {
const byte REG_ADDR = 0X0F;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_N_CHANNELS > 6 // ADS1298 or ADS1298R
const byte B_LOFF8P = 0x80 | RESERVED_BITS;
const byte B_LOFF7P = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_LOFF6P = 0x20 | RESERVED_BITS;
const byte B_LOFF5P = 0x10 | RESERVED_BITS;
#endif
const byte B_LOFF4P = 0x08 | RESERVED_BITS;
const byte B_LOFF3P = 0x04 | RESERVED_BITS;
const byte B_LOFF2P = 0x02 | RESERVED_BITS;
const byte B_LOFF1P = 0x01 | RESERVED_BITS;
}

namespace loffSensn {
const byte REG_ADDR = 0X10;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_N_CHANNELS > 6 // ADS1298 or ADS1298R
const byte B_LOFF8N = 0x80 | RESERVED_BITS;
const byte B_LOFF7N = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_LOFF6N = 0x20 | RESERVED_BITS;
const byte B_LOFF5N = 0x10 | RESERVED_BITS;
#endif
const byte B_LOFF4N = 0x08 | RESERVED_BITS;
const byte B_LOFF3N = 0x04 | RESERVED_BITS;
const byte B_LOFF2N = 0x02 | RESERVED_BITS;
const byte B_LOFF1N = 0x01 | RESERVED_BITS;
}

namespace loffFlip {
const byte REG_ADDR = 0X11;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_N_CHANNELS > 6 //  ADS1298 or ADS1298R
const byte B_LOFF_FLIP8 = 0x80 | RESERVED_BITS;
const byte B_LOFF_FLIP7 = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_LOFF_FLIP6 = 0x20 | RESERVED_BITS;
const byte B_LOFF_FLIP5 = 0x10 | RESERVED_BITS;
#endif
const byte B_LOFF_FLIP4 = 0x08 | RESERVED_BITS;
const byte B_LOFF_FLIP3 = 0x04 | RESERVED_BITS;
const byte B_LOFF_FLIP2 = 0x02 | RESERVED_BITS;
const byte B_LOFF_FLIP1 = 0x01 | RESERVED_BITS;
}

namespace loffStatp {
const byte REG_ADDR = 0X12;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = true;

#if ADS_N_CHANNELS > 6 // ADS1298 or ADS1298R
const byte B_IN8P_OFF = 0x80 | RESERVED_BITS;
const byte B_IN7P_OFF = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_IN6P_OFF = 0x20 | RESERVED_BITS;
const byte B_IN5P_OFF = 0x10 | RESERVED_BITS;
#endif
const byte B_IN4P_OFF = 0x08 | RESERVED_BITS;
const byte B_IN3P_OFF = 0x04 | RESERVED_BITS;
const byte B_IN2P_OFF = 0x02 | RESERVED_BITS;
const byte B_IN1P_OFF = 0x01 | RESERVED_BITS;
}

namespace loffStatn {
const byte REG_ADDR = 0X13;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = true;

#if ADS_N_CHANNELS > 6 //  ADS1298 or ADS1298R
const byte B_IN8N_OFF = 0x80 | RESERVED_BITS;
const byte B_IN7N_OFF = 0x40 | RESERVED_BITS;
#endif
#if ADS_N_CHANNELS > 4 // ADS1296, ADS1296R , ADS1298 or ADS1298R
const byte B_IN6N_OFF = 0x20 | RESERVED_BITS;
const byte B_IN5N_OFF = 0x10 | RESERVED_BITS;
#endif
const byte B_IN4N_OFF = 0x08 | RESERVED_BITS;
const byte B_IN3N_OFF = 0x04 | RESERVED_BITS;
const byte B_IN2N_OFF = 0x02 | RESERVED_BITS;
const byte B_IN1N_OFF = 0x01 | RESERVED_BITS;
}

namespace gpio {
const byte REG_ADDR = 0X14;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x0F;

const byte B_GPIOD4 = 0x80;
const byte B_GPIOD3 = 0x40;
const byte B_GPIOD2 = 0x20;
const byte B_GPIOD1 = 0x10;
const byte B_GPIOC4 = 0x08;
const byte B_GPIOC3 = 0x04;
const byte B_GPIOC2 = 0x02;
const byte B_GPIOC1 = 0x01;
}

namespace pace {
const byte REG_ADDR = 0X15;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

const byte B_PACEE1 = 0x10;
const byte B_PACEE0 = 0x08;
const byte B_PACEO1 = 0x04;
const byte B_PACEO0 = 0x02;
const byte B_PDB_PACE = 0x01;

const byte PACEE_CHAN2 = 0x00 | RESERVED_BITS;
const byte PACEE_CHAN4 = B_PACEE0 | RESERVED_BITS;
const byte PACEE_CHAN6 = B_PACEE1 | RESERVED_BITS;
const byte PACEE_CHAN8 = (B_PACEE1 | B_PACEE0 | RESERVED_BITS);

const byte PACEO_CHAN1 = 0x00 | RESERVED_BITS;
const byte PACEO_CHAN3 = B_PACEO0 | RESERVED_BITS;
const byte PACEO_CHAN5 = B_PACEO1 | RESERVED_BITS;
const byte PACEO_CHAN7 = (B_PACEO0 | B_PACEO1 | RESERVED_BITS);
}

namespace resp {
const byte REG_ADDR = 0x16;
// FIXME: the reset value is 0x00 but datasheet says "Always write 1" in bit 5. I check in ADS1294 that the reset value is 0x00. Check it (page 80) when a new version of the datasheet will be available
const byte RESERVED_BITS = 0x20;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_HAS_RESPIRATION_MODULE
const byte B_RESP_DEMOD_EN1 = 0x80;
const byte B_RESP_MOD_EN1 = 0x40;
#endif
const byte B_RESP_PH2 = 0x10;
const byte B_RESP_PH1 = 0x08;
const byte B_RESP_PH0 = 0x04;
const byte B_RESP_CTRL1 = 0x02;
const byte B_RESP_CTRL0 = 0x01;

const byte RESP_PH_22_5 = 0x00 | RESERVED_BITS;
const byte RESP_PH_45 = B_RESP_PH0 | RESERVED_BITS;
const byte RESP_PH_67_5 = B_RESP_PH1 | RESERVED_BITS;
const byte RESP_PH_90 = (B_RESP_PH1 | B_RESP_PH0 | RESERVED_BITS);
const byte RESP_PH_112_5 = B_RESP_PH2 | RESERVED_BITS;
const byte RESP_PH_135 = (B_RESP_PH2 | B_RESP_PH0 | RESERVED_BITS);
const byte RESP_PH_157_5 = (B_RESP_PH2 | B_RESP_PH1 | RESERVED_BITS);

const byte RESP_NONE = 0x00 | RESERVED_BITS;
const byte RESP_EXT = B_RESP_CTRL0 | RESERVED_BITS;

#if ADS_HAS_RESPIRATION_MODULE
const byte RESP_INT_SIG_INT = B_RESP_CTRL1 | RESERVED_BITS;
const byte RESP_INT_SIG_EXT = (B_RESP_CTRL1 | B_RESP_CTRL0 | RESERVED_BITS);
#endif
}

namespace config4 {
const byte REG_ADDR = 0X17;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

const byte B_RESP_FREQ2 = 0x80;
const byte B_RESP_FREQ1 = 0x40;
const byte B_RESP_FREQ0 = 0x20;
const byte SINGLE_SHOT = 0x08;
const byte WCT_TO_RLD = 0x04;
const byte PD_LOFF_COMP = 0x02;

const byte RESP_FREQ_64k_Hz = 0x00 | RESERVED_BITS;
const byte RESP_FREQ_32k_Hz = B_RESP_FREQ0 | RESERVED_BITS;
const byte RESP_FREQ_16k_Hz = B_RESP_FREQ1 | RESERVED_BITS;
const byte RESP_FREQ_8k_Hz = (B_RESP_FREQ1 | B_RESP_FREQ0 | RESERVED_BITS);
const byte RESP_FREQ_4k_Hz = B_RESP_FREQ2 | RESERVED_BITS;
const byte RESP_FREQ_2k_Hz = (B_RESP_FREQ2 | B_RESP_FREQ0 | RESERVED_BITS);
const byte RESP_FREQ_1k_Hz = (B_RESP_FREQ2 | B_RESP_FREQ1 | RESERVED_BITS);
const byte RESP_FREQ_500_Hz = (B_RESP_FREQ2 | B_RESP_FREQ1 | B_RESP_FREQ0 | RESERVED_BITS);
}

namespace wct1 {
const byte REG_ADDR = 0X18;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

#if ADS_N_CHANNELS > 6
const byte B_aVR_CH7 = 0x20;
#endif
#if ADS_N_CHANNELS > 4 
const byte B_aVF_CH6 = 0x80;
const byte B_aVL_CH5 = 0x40;
#endif
const byte B_avR_CH4 = 0x10;
const byte B_PD_WCTA = 0x08;
const byte B_WCTA2 = 0x04;
const byte B_WCTA1 = 0x02;
const byte B_WCTA0 = 0x01;

const byte WCTA_CH1P = 0x00 | RESERVED_BITS;
const byte WCTA_CH1N = B_WCTA0 | RESERVED_BITS;
const byte WCTA_CH2P = B_WCTA1 | RESERVED_BITS;
const byte WCTA_CH2N = (B_WCTA1 | B_WCTA0 | RESERVED_BITS);
const byte WCTA_CH3P = B_WCTA2 | RESERVED_BITS;
const byte WCTA_CH3N = (B_WCTA2 | B_WCTA0 | RESERVED_BITS);
const byte WCTA_CH4P = (B_WCTA2 | B_WCTA1 | RESERVED_BITS);
const byte WCTA_CH4N = (B_WCTA2 | B_WCTA1 | B_WCTA0 | RESERVED_BITS);
}

namespace wct2 {
const byte REG_ADDR = 0X19;
const byte RESERVED_BITS = 0x00;
const boolean READ_ONLY_REGISTER = false;
const byte RESET_VALUE = 0x00;

const byte B_PD_WCTC = 0x80;
const byte B_PD_WCTB = 0x40;
const byte B_WCTB2 = 0x20;
const byte B_WCTB1 = 0x10;
const byte B_WCTB0 = 0x08;
const byte B_WCTC2 = 0x04;
const byte B_WCTC1 = 0x02;
const byte B_WCTC0 = 0x01;

const byte WCTB_CH1P = 0x00 | RESERVED_BITS;
const byte WCTB_CH1N = B_WCTB0 | RESERVED_BITS;
const byte WCTB_CH2P = B_WCTB1 | RESERVED_BITS;
const byte WCTB_CH2N = (B_WCTB1 | B_WCTB0 | RESERVED_BITS);
const byte WCTB_CH3P = B_WCTB2 | RESERVED_BITS;
const byte WCTB_CH3N = (B_WCTB2 | B_WCTB0 | RESERVED_BITS);
const byte WCTB_CH4P = (B_WCTB2 | B_WCTB1 | RESERVED_BITS);
const byte WCTB_CH4N = (B_WCTB2 | B_WCTB1 | B_WCTB0 | RESERVED_BITS);

const byte WCTC_CH1P = 0x00 | RESERVED_BITS;
const byte WCTC_CH1N = B_WCTC0 | RESERVED_BITS;
const byte WCTC_CH2P = B_WCTC1 | RESERVED_BITS;
const byte WCTC_CH2N = (B_WCTC1 | B_WCTC0 | RESERVED_BITS);
const byte WCTC_CH3P = B_WCTC2 | RESERVED_BITS;
const byte WCTC_CH3N = (B_WCTC2 | B_WCTC0 | RESERVED_BITS);
const byte WCTC_CH4P = (B_WCTC2 | B_WCTC1 | RESERVED_BITS);
const byte WCTC_CH4N = (B_WCTC2 | B_WCTC1 | B_WCTC0 | RESERVED_BITS);
}

} // End of the register namespace
} // End of the ads namespace

#endif /* ADS129XX_CONSTANTS_H_ */




