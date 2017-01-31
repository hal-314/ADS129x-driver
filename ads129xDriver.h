/*  
    ---------------------------------
    
    ADS model, bits per channel and other configurations goes to ads129xDriverConfig.h file.
    This file is only for code.

    Constants present in the Datasheet and some helper constant to configure the registers in ADS chip are defined in ads129xDatashhetConstants.h
    Read the documentation in these file to know more about it.
    
    --------------------------------

    Arduino library for ADS1294, ADS1294R, ADS1296, ADS1296R, ADS1298 and ADS1298R chips.
    All these chips share the same datasheet "ADS129x Low-Power, 8-Channel, 24-Bit Analog Front-End for Biopotential Measurements"
    
    Typical usage (see adsDriverExample for an example):
      0- Configure the ADS to use, verbose level and bits per channel (is not the same than resolution) in ads129xDriverConfig
      1- Cal1 the constructor
      2- Call begin method
      3- (Optional) Configure ADS with read/write register methods and related helper methods ({enable/disable}Channel, ...)
      4- Put in Start mode the ADS by sendSPICommandSTART or enableHardwareStartMode methods
      5- Put ADS in RDATA(read one new sample) or RDATAC (read continuously new sample) mode 
      6- Check if new data is available with hasNewDataAvailable method
      7- If so, recover the new data sent by ADS with getData method



    The data format returned by getData is an union named ads_data_t with the same size that the data sent by ADS:
    
      typedef union {
        byte rawData[_ADS_DATA_PACKAGE_SIZE]; // Max possible size (ADS1298 in 24 bit per channel) is : 24 status bits + 24 bits per channel × 4 channels = 216 -> 27 bytes.
        struct {
          byte statusWord[3];
          ads_bits_sample_t channel[ADS_N_CHANNELS]; // ADS_N_CHANNELS is constant generated from ADS chip specified
        } formatedData;
      } ads_data_t;

      ads_bits_sample_t is also an enum that depends with the value of ADS_BITS_PER_CHANNEL constant selected by user (it is defined in ads129xDriverConfig.h)

      If ADS_BITS_PER_CHANNEL is 16, then:
      
          typedef struct ads_bits_sample_t {
            uint8_t hi, low; // High and low byte. It's equivalent to an int of 16 bits.
          } ads_bits_sample_t;
          
      Otherwise: 
      
          typedef struct ads_bits_sample_t {
            uint8_t hi, mid, low; // High, middle and low byte. It's equivalent to an int of 24 bits 
          } ads_bits_sample_t;

          

    The main limitation of this library is that doesn't implement any buffer for data sent by ADS chip. So, if you don't call 
    getData and copy the data to another memory location, the next data retrieval from ADS will overwrite he old. See Limitations 
    section to know about the other few limitations that has the library.

    
    To know which is the minimum SPI speed you need, see page 59, section 9.5.1.2 Serial Clock (SCLK), in the datasheet:
        Minimum SPI speed (in MHz) = 1/((T_sampling - 8 * t_clk)/(Nbits * Nchannels + 24))/1e6 

        T_sampling: signal sample frequency 
        T_clk: ADS clock (typically 2.048 MHz)
        Nbits: number the bits per channels sent by ADS (is the same that ADS_BITS_PER_CHANNEL constant)
        Nchannels: number of channels that has the ADS chip

    Be careful that this formula suppose that their is no delay in the SPI master to operate SPI. It isn't the case for Arduino boards
     
    For the development of this library, the revision K (August 2015) of the datasheet was used.
    
    Limitations:
      1- It hasn't a buffer to save the need data sent by ADS. New data will always overwrite the old data.
      2- Doesn't support send multibyte commands using burst method (see page 63, section 9.5.2.9 Sending Multibyte Commands,
         in the datasheet). See note below.
      3- Doesn't support multiread and multiwrite registers but their implementation is quite trivial. I you would want to
         support them, you need to modify readRegister() and writeRegister() methods.
      4- The maximum SPI speed is 4 MHz instead the theoretical 15/20 MHz due to the lack of the support for multibyte commands using 
         burst method. See note below. This speed might not be enough for ADS1298 or ADS1298R with high sampling rates. You can use 
         the formula gave above to compute the minimum SPI speed you need.
      5- Doesn't support multiple device configuration due to (I believe that there are this only two limitations):
           1- Method that reads the data converted from ADS doesn't support when 2 o more ADS are in Daisy-Chain configuration.
           2- The workaround for interruptions limit to one the active ADS that can be controlled. So, Cascade configuration is not supported.

           See pages 56 and 57, section 9.4.2 Multiple-Device Configuration, in datsheet for more infromation about Mutiple-device Configuration

      Note: implementing burst method and therefore increasing the maximum SPI speed has little sense when Arduino libraries are used. Burst method requiere a high
            time resolution (optimally 0.1 microseconds) that standard Arduino library doesn't provide. However, SPI max speed for data reading in continuous mode could be increased
            if different SPISettings are used to send commands and to read the data sent in the continuous mode (in this mode, no commands are sent to retrieve data from ADS chip).
            However, firstly Arduino boards need to support high SPI transfers. For example, the chip in Arduino zero boards (they use a SAMD21 chip (an ARM M0 microprocessor) allows
            to 24 MHz but it is not recommend to use more than 12 MHz if the peripheral is connected with wires.

            To sum up, increase theoretical maximum SPI speed doesn't seem apport much.

    Credits:    
      - This library is inspired by ADS129x-tools (https://github.com/adamfeuer/ADS129x-tools)
      - Antonio and Marc for the support given during the development
      
*/

#ifndef _ADS129X_DRIVER_H_
#define _ADS129X_DRIVER_H_

#include <Arduino.h>

/* ======= ADS constants section ============= */
// Import user configuration where ADS_BITS_PER_CHANNEL, ADS_N_CHANNELS, ADS_HAS_RESPIRATION_MODULE and ADS_LIBRARY_VERBOSE_LEVEL are defined
#include "ads129xDriverConfig.h"

// Check that all constants are defined by user
#if !defined (ADS_BITS_PER_CHANNEL) || !defined (ADS_N_CHANNELS) || !defined (ADS_HAS_RESPIRATION_MODULE) || !defined (ADS_LIBRARY_VERBOSE_LEVEL)
Some constants that are defined in file "ads129xDriverConfig.h" are not defined!!!! To user: Do you delete some constants ?
#endif

// To avoid pollute this header with an endless constants related to registers and commands for ADS129x chips, these are declared in ads129xxConstants.h
#include "ads129xDatasheetConstants.h"

// Time constants. See section 7.6, Timing Requirements: Serial Interface in the datasheet
// In the datasheet, they use nominal _ADS_T_CLK -> _ADS_T_CLK = 1/2.048MHz
#define _ADS_T_CLK 0.514 // See page 17, section 9 7.6 Timing Requirements: Serial Interface, in the datasheet. Note: if internal clock is used, max _ADS_T_CLK is 500 ns and then _ADS_T_CLK_2 could be 1, _ADS_T_CLK_4 could be 2 and _ADS_T_CLK_18 could be 9
#define _ADS_T_CLK_2 ceil(2 * _ADS_T_CLK) // 2 *T _CLK = 1.028 ms. delayMicroseconds function only accept integers. In order to be make sure that the code will always for any chip configuration -> ceil float to the next integer  
#define _ADS_T_CLK_4 ceil(4 * _ADS_T_CLK) // 4 *T _CLK = 2.056 ms. delayMicroseconds function only accept integers. In order to be make sure that the code will always for any chip configuration -> ceil float to the next integer  
#define _ADS_T_CLK_18 ceil(18 * _ADS_T_CLK) // 18 * _ADS_T_CLK = 9.252 ms. delayMicroseconds function only accept integers. In order to be make sure that the code will always for any chip configuration -> ceil float to the next integer  

// Timing requeriments for SPI interface. These are ignored because there are SPI transactions inside interruptions -> 
// delay doesn't work and arduino are slow enough that is not necessary to wait any time.
// Tested in Arduino M0
// #define _ADS_T_CSSC 1 // Is 6 or 17 ns but arduino doesn't have a function to wait nanoseconds -> to make sure will wait 1 microsecond.
// #define _ADS_T_SCCS _ADS_T_CLK_4 // aprox. 2 microseconds
// #define _ADS_T_CSH _ADS_T_CLK_2 // aprox. 1 microsecond

// SPI constants.
/*
  Max SPI speed is 15/20 MHz (depends if ADS12XX is powered by less/more than 2V) but the code doesn't support send multibyte
  commands using burst method (see page 63, section 9.5.2.9 Sending Multibyte Commands, in the datasheet). So, 4 MHz is the max
  SPI speed achievable.

  Side note: I think that with arduino libraries, burst method has little performance gain due to 0.1 microseconds time resolution
  is needed to achieve optimal performance.
  For more information about SPI max speed, see page 17 in the datasheet
*/
#define _ADS_SPI_MAX_SPEED 4e6 // 4 MHz
#define _ADS_SPI_BIT_ORDER MSBFIRST // See page 64, section 9.5.2.10 RREG: Read From Register, in the datasheet)
#define _ADS_SPI_MODE SPI_MODE1 // See page 17 in the datasheet

// If VCAP1 is not an issue, t_por allows us to wait only 150 ms BUT I didn't calculate VCAP1 time. See page 96 in the datasheet
// To make sure that VCAP1 won't be an issue, we wait 1 second. If you use the recomended capacitor for VCAP1 pin (22 micro Faradays), I think 150 ms is enough
#define _ADS_POWER_UP_DELAY_MS 1000 // Wait time after device is powered up until commands are sent

#define _ADS_READING_DATA_IN_RDATA_MODE 1
#define _ADS_READING_DATA_IN_RDATAC_MODE 2
#define _ADS_NO_READING_NEW_DATA 3

#define ADS_PIN_NOT_USED 255 // Max posible value that can take a uint8_t type
/* ======= ads_data_t definition  ============= */

// The size of the data sent by ads129xx depends by the number of bits per channel and the number of channels in the chip.
#if ADS_BITS_PER_CHANNEL == 16
#define _ADS_DATA_PACKAGE_SIZE (3 + 2 * ADS_N_CHANNELS)
typedef struct ads_bits_sample_t {
  uint8_t hi, low;
} ads_bits_sample_t;

#elif ADS_BITS_PER_CHANNEL == 24

#define _ADS_DATA_PACKAGE_SIZE (3 + 3 * ADS_N_CHANNELS)
typedef struct ads_bits_sample_t {
  uint8_t hi, mid, low;
} ads_bits_sample_t;
#endif

// Generic union for data receviced from any ADS129xx chip
typedef union {
  byte rawData[_ADS_DATA_PACKAGE_SIZE]; // Max size (ADS1298 in 24 bit per channel): 24 status bits + 24 bits per channel × 4 channels = 216 -> 27 bytes.
  struct {
    byte statusWord[3];
    ads_bits_sample_t channel[ADS_N_CHANNELS];
  } formatedData;
} ads_data_t;

/* ======= ADS129xSensor class definition  ============= */
class ADS129xSensor {
  private:
    volatile boolean isSpiOpen, hasNewData;
    volatile uint8_t readingStatus;
    uint8_t chipSelectPin, drdyPin, resetPin, startPin, pwdnPin, clkselPin;

    // It is declarated in order to allocate memory and avoid to allocate every time that new data is available
    // In the worst case scenario, it takes 27 bytes ( in ADS1298 or ADS1298R model with 24 bits resolution).
    ads_data_t adsData; // Use constant _ADS_DATA_PACKAGE_SIZE to know how many bytes has the data sent by ADS chip

    /* ==== Methods ===== */
    void beginSpiTransaction();
    void endSpiTransaction();

    // Low level function. It only send command without any knowloadge of timing restrictions for
    // specific command.
    void sendCommand(byte command, boolean keepSpiOpen = false);
    void resetADS();
    
  public:
    // For limitations in attachInterrupt and the workaround, this function must be public but YOU MUST NOT USE IT
    // Read the new data from ADS when ADS indicate that new data is available. This method is called inside an interruption
    void _privateReadDataFromChip_();

  public:
    // If you don't want to use an optional pin, you can pass the constant ADS_PIN_NOT_USED
    // See Limitations section in the top of this file to know the limitations that has this library
    // Before using any othr method, you MUST call begin() function
    ADS129xSensor(uint8_t chipSelectPin, uint8_t drdyPin, uint8_t resetPin = ADS_PIN_NOT_USED, uint8_t startPin = ADS_PIN_NOT_USED,
                  uint8_t pwdnPin = ADS_PIN_NOT_USED, uint8_t clkselPin = ADS_PIN_NOT_USED) {
      this->chipSelectPin = chipSelectPin;
      this->drdyPin = drdyPin;
      this->resetPin = resetPin;
      this->startPin = startPin;
      this->pwdnPin = pwdnPin;
      this->clkselPin = clkselPin;
      isSpiOpen = false;
      hasNewData = false;
      readingStatus = _ADS_NO_READING_NEW_DATA;
    };
    ~ADS129xSensor() {};

    // Configure pins and interrupts, performs the chip power-up, leave
    // it in in default reset options and ready to accept commands or read/write register
    // or start conversion if START command is send.
    // Be aware that:
    //    - The data conversion is stopped.
    //    - SDATAC commnand was sent to let register configurations.
    //    - If clkselPin was provided in the constructor, it configure ADS chip to use the external clock.
    //
    // See page 65 in the datasheet for more information about which are the registers reset values
    void begin();
    // Call to finish all data conversion and release ADS. Then you can control another ADS chip calling the begin() method.
    // GPIO pins are also released. Call begin method to use ADS again
    void end();

    // Return the data sent by ADS and mark the data as no new (hasNewDataAvailable will return false until the next data sent
    // by ADS is received). Be aware that there is no buffer, so new data override the old.
    ads_data_t * getData() {
      hasNewData = false; // Mark adsData as read
      return &adsData;
    }

    // Method to check if new data is available.
    boolean hasNewDataAvailable() volatile {
      return hasNewData;
    }


    /* ====== Methods that use hardware pins ========== */
    // Reset the ADS using RESET pin
    // Only it can be used if reset pin is specified
    void doHardwareReset();
    // Put the ADS in start mode using START pin
    // Only it can be used if start pin is specified
    void enableHardwareStartMode();
    // Stop ADS start mode using START pin
    // Only it can be used if start pin is specified
    void disableHardwareStartMode();
    // Use CLKSEL pin to select that clock source for ADS will be EXTERNAL
    // Only it can be used if clksel pin is specified
    void enableExternalClockSource();
    // Use CLKSEL pin to select that clock source for ADS will be INTERNAL
    // Only it can be used if clksel pin is specified
    void disableExternalClockSource();
    // Put all ADS on-chip circuitry powered down using PDWN pin
    // Only it can be used if pwdn pin is specified
    void enableHardwarePowerDownMode();
    // Exit the state that all ADS on-chip circuitry are powered down. It use PDWN pin.
    // Only it can be used if pwdn pin is specified
    void disableHardwarePowerDownMode();


    /* =====  Register related methods  ====== */
    // Read the ADS register pointed by registAddr. See know about keepSpiOpen, read the comment
    // about methods related to send SPI commands (it's the next section)
    // Return 0xFF if read SPI command can't be sent due to ADS is in RDATAC mode. Otherwise, 
    // return the register value
    byte readRegister(byte registAddr, boolean keepSpiOpen = false);

    // WARNING: when you write a register, you write in all the bits of the register.
    // So, if the register has already configurated, you will lost this configuration
    // when you write again in it. So, I recommend write all configuration at once.
    //
    // Example:
    //    NOT to do:
    //      writeRegister(ads::registers::chnSet::REG_ADDR_CH1SET, ads::registers::chnSet::DISABLE_CHANNEL); // This write 1 in bit PDn (it's pointed by constant B_PDn)
    //      writeRegister(ads::registers::chnSet::REG_ADDR_CH1SET, ads::registers::chnSet::GAIN_6X); // You are writting zeros the others non reserved bits --> you are enabling this channel !!!!
    //    DO instead:
    //      writeRegister(ads::registers::chnSet::REG_ADDR_CH1SET, ads::registers::chnSet::DISABLE_CHANNEL | ads::registers::chnSet::GAIN_6X);
    void writeRegister(byte registAddr, byte value, boolean keepSpiOpen = false);
    void setAllRegisterToResetValuesWithoutResetCommand(boolean keepSpiOpen = false);


    /* ======= Methods related to send ADS commands (except read/write registers) ============= */
    // To send SPI command to ADS, you must use the method sendSPICommand{Command name in datasheet}(boolean keepSpiOpen = false)
    // Commands available in ADS12XX (see page 61, section 9.5.2 SPI Command Definitions, in the datasheet)
    //
    // If the argumment keepSpiOpen is true, SPI communication with chip is left open (SPI
    // bus can't be used be other peripherals and no new data is received from ADS) and ready
    // to send more SPI commands/ read or write registers to the chip. Then, the following commands
    // or read/write registers are sent more quickly.
    //
    // You might use keepSpiOpen = true when you can't spend much time to configure the chip. For example, you are in the middle of a
    // measurement and you want to power down two channels (write two different registers) due to electrode disconnection. In this
    // situation, you  want to configure the ADS as fast as possible in order to not loss any sample (or minimize the samples loss).
    // So, using keepSpiOpen = true, you will avoid to open and close SPI communication between consecutive register writings.
    //
    // If time is not critical (no matter how many miliseconds you waste configurating ADS), it won't be benefical to use the methods
    // with keepSpiOpen = true.
    //
    // Be aware (it's said above) that no new data will be receive until you send command with keepSpiOpen = false
    // receive more SPI commands or read/write register. For that reason,
    //
    // Finally, time restriction specific for each SPI command sent is taken in acccount. So, you don't need to worry to,
    // for example, wait 4 clock periods before send another SPI command after sent WAKEUP command.


    void sendSPICommandWAKEUP(boolean keepSpiOpen = false);
    void sendSPICommandSTANDBY(boolean keepSpiOpen = false);

    // Be careful! It can't be used if reset pin is specified
    void sendSPICommandRESET(boolean keepSpiOpen = false);
    // Be careful! It can't be used if start pin is specified
    void sendSPICommandSTART(boolean keepSpiOpen = false);
    // Be careful! It can't be used if start pin is specified
    void sendSPICommandSTOP(boolean keepSpiOpen = false);

    // Be aware that when RDATAC command is sent, the any other command except SDATAC will be ignored
    void sendSPICommandRDATAC(boolean keepSpiOpen = false);
    void sendSPICommandSDATAC(boolean keepSpiOpen = false);    
    // Be careful! This method tells the ADS driver to issue the RDATA SPI command and read the sample 
    // when DRDY pin goes low. So you need to worry if DRDY is low or not. New data will be available
    // the next time thats DRDY goes low.
    void sendSPICommandRDATA(boolean keepSpiOpen = false);


    /* ======= Class methods class implementing typical ADS configurations  ============= */

    // Only disable ECG channel, without changing the other bits (so, without changing the other configuration)
    // Texas Instruments recommends to short the inputs when power down a channel. So setInputAsShorted argument
    // is facilitate.
    //
    // If setInputAsShorted is set to true, inputs are shorted. Remember to configure the channel input
    // again when you are going to enable the channel is you use setInputAsShorted = true.
    void disableChannel(uint8_t nChannel, boolean setInputAsShorted, boolean keepSpiOpen = false);

    // Only enable ECG channel, without changing the other bits (so, without changing the other configuration).
    // You can change the channel input with channelInput argument. It expects -1 or the following
    // ads::registers::chnSet constant: ELECTRODE_INPUT, SHORTED, RLD_INPUT, MVDD, TEMP, TEST_SIGNAL and
    // RLD_DRP, RLD_DRN. When channelInput is -1, current channelInput configuration is kept.
    void enableChannel(uint8_t nChannel, int8_t channelInput = -1, boolean keepSpiOpen = false);

    // Only enable ECG channel and set the gain but without changing the other bits (so, without changing the other configuration)
    // You can change the channel input with channelGainConstant argument. It expects an ads::registers::CHnSet::GAIN_XX constant.
    // See documentation of enableChannel(uint8_t nChannel, int8_t channelInput = 0xFF, boolean keepSpiOpen = false); for
    // the usage of channelInput argument
    void enableChannelAndSetGain(uint8_t nChannel, byte channelGainConstant, int8_t channelInput = -1, boolean keepSpiOpen = false);
};

#endif /* _ADS129X_DRIVER_H__ */
