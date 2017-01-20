/*
 * In this file, user must edit the constants ADS_CHIP_USED and ADS_BITS_PER_CHANNEL (lines 20 and 30) to set
 * which ADS chip will be used and how many bits wants per channel.  
 * 
 * In your code, ADS_BITS_PER_CHANNEL, ADS_N_CHANNELS and ADS_HAS_RESPIRATION_MODULE will be available if 
 * you have in mind to change ADS chip. See lines 30 and 42 for the definitions of them
 */
#ifndef _ADS129X_DRIVER_CONFIG_H_
#define _ADS129X_DRIVER_CONFIG_H_

// ADS supported chips:
#define ADS_1294 1
#define ADS_1294R 2
#define ADS_1296 3
#define ADS_1296R 4
#define ADS_1298 5
#define ADS_1298R 6

/* ============ User editable ============= */
#define ADS_CHIP_USED ADS_1294 // Put here the chip used. You have above definitions to help you. For example, here I chose to use ADS1294 chip 

// Number of bits per channel that ADS chip will sent over SPI. It doesn't mean that sample was adquiered with this resolution. 
// The number of bits per channel depends with data rate (sample frequency). See page 53, section 9.4.1.3.2 Readback length in 
// the datasheet for more information
// For 32 or 64 kSPS (kilo sample per second) data rate, use 16. For all another data rates (from 250 SPS to 16 kSPS), use 24
//
// Note: Datasheet says in 32 kSPS and 64 kSPS, the bits per channel send by ADS is 16 BUT ADS max sample frequency is 32 kSPS. 
//       I think that maybe datasheet wanted to say 16 kSPS instead of 64 kSPS. I don't know. I used the datasheet, revision K as reference. 
//       If datasheet is wrong, in ads129xDatasheetConstants.h is incorrect (look for usage of ADS_BITS_PER_CHANNEL)   
#define ADS_BITS_PER_CHANNEL 24 // Could be 16 or 24. 

// This define specifies how much verbosity you want when you use the library. It can take the following valuesre:
//    0 -> No messages (recomended if memory program is critical. It avoids to import Serial library and send message through Serial.print() )
//    1-> only errors and a few selected messages (recommended)
//    2-> all (for debug purposes only)
#define ADS_LIBRARY_VERBOSE_LEVEL 1 // 0, 1, 2




/* ============== DON'T TOUCH =========== */
// For each ADS type, we define the number of the channels (ADS_N_CHANNELS) and if has integrated the respiration module to 
// mesure respiration without external hardware (ADS_HAS_RESPIRATION_MODULE).

#if ADS_CHIP_USED == ADS_1294 
#define ADS_N_CHANNELS 4 // Number of channels that ADS has. Could be 4, 6 or 8 
#define ADS_HAS_RESPIRATION_MODULE false // If ADS has the respiration module (if its coding ends with R or not). 

#elif ADS_CHIP_USED == ADS_1294R 
#define ADS_N_CHANNELS 4 // Number of channels that ADS has. Could be 4, 6 or 8 
#define ADS_HAS_RESPIRATION_MODULE true // If ADS has the respiration module (if its coding ends with R or not). 

#elif ADS_CHIP_USED == ADS_1296
#define ADS_N_CHANNELS 6 // Number of channels that ADS has. Could be 4, 6 or 8 
#define ADS_HAS_RESPIRATION_MODULE false // If ADS has the respiration module (if its coding ends with R or not). 

#elif ADS_CHIP_USED == ADS_1296R 
#define ADS_N_CHANNELS 6 // Number of channels that ADS has. Could be 4, 6 or 8 
#define ADS_HAS_RESPIRATION_MODULE true // If ADS has the respiration module (if its coding ends with R or not). 

#elif ADS_CHIP_USED == ADS_1298
#define ADS_N_CHANNELS 8 // Number of channels that ADS has. Could be 4, 6 or 8 
#define ADS_HAS_RESPIRATION_MODULE false // If ADS has the respiration module (if its coding ends with R or not). 

#elif ADS_CHIP_USED == ADS_1298R 
#define ADS_N_CHANNELS 8 // Number of channels that ADS has. Could be 4, 6 or 8 
#define ADS_HAS_RESPIRATION_MODULE true // If ADS has the respiration module (if its coding ends with R or not). 

#else
"The value set in ADS_CHIP_USED is not in ADS supported chips defines!!!!!!"
#endif 

#endif /* _ADS129X_DRIVER_CONFIG_H_ */
