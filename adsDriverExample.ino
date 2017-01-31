#include <Arduino.h>

#include "ads129xDriver.h"


// The driver is configured to take control of an ADS1294 chip.
// In file ads129xDriverConfig.h you have to specify the model of the ADS, bits per channel 
// and the level of verbose of the library.

// GPIO pins where CS, DRDY and RESET pins of the ADS chip are conected.
/* ============== GPIO pins connected to ADS chip ===========*/
// CS and DRDY pin of the ADS chip MUST be connected to Arduino
#define ADS_CS_PIN 6 // CS pin is mandatory
#define ADS_DRDY_PIN 5 // DRDY pin is mandatory
// Optional pins connections. They usually doesn't add new functionality but they provide a way 
// to avoid to use SPI commands to achieve the desired functionality.
// RESET pins connection is optional but recomended. If ADS doesn't respond to SPI commands due to a malfunction, RESET pin always work.
#define ADS_RESET_PIN 10 // If it not used, it could be the constant ADS_PIN_NOT_USED or call the constructor with only two arguments

// Constructor
ADS129xSensor adsSensor(ADS_CS_PIN, ADS_DRDY_PIN, ADS_RESET_PIN);
// ADS129xSensor adsSensor(ADS_CS_PIN, ADS_DRDY_PIN); // Only CS and DRDY pins are used
// ADS129xSensor adsSensor(ADS_CS_PIN, ADS_DRDY_PIN, ADS_PIN_NOT_USED); // // Only CS and DRDY pins are used. Equivalent to the previous line

// Helper function to print byte value in bits
void printBits(byte myByte) {
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

void setup() {
  // put your setup code here, to run once:
  while (!Serial);
  Serial.begin(115200);

  Serial.println("Starting the ADS setup");

  Serial.println("Calling ADS129xSensor.begin() method. Remember to call at the beginning");
  // You MUST call begin() method before any other method of ADS129xSensor
  adsSensor.begin();

  // It is not need. Only for example purposes
  Serial.println("Example: reset value for all registers without reset command");
  adsSensor.setAllRegisterToResetValuesWithoutResetCommand();

  // Read ADS129x ID:
  byte regValue = adsSensor.readRegister(ads::registers::id::REG_ADDR);

  Serial.print("ID register: ");
  printBits(adsSensor.readRegister(ads::registers::id::REG_ADDR));
  Serial.println("");

  Serial.print("Set sampling read to 1 kHz and low-power mode");
  Serial.print("Keep in mind that when config1 or resp registers are changed, internal reset is performed. See the datasheet, section Reset");
  // By default, ADS12xx is in low-power consumption and with a sample frequency of 250 Hz
  adsSensor.writeRegister(ads::registers::config1::REG_ADDR, ads::registers::config1::LOW_POWR_1k_SPS);
  Serial.print("The new value CONFIG1 register is ");
  printBits(adsSensor.readRegister(ads::registers::config1::REG_ADDR));

  // Setup of my circuit. In my case, it hadn't external reference,
  Serial.println("Enabling internal reference buffer --> set PD_REFBUF to 1");
  // If you change inividual bits with constants B_xx, you must add with the RESERVED_BITS constant value to be sure that you will
  // write the right bits in the reserved bits in the register.
  // Remember to write all desired configuration in a register  simultaneously. When you write a register, you delete all previous values
  adsSensor.writeRegister(ads::registers::config3::REG_ADDR, ads::registers::config3::B_PD_REFBUF | ads::registers::config3::RESERVED_BITS);
  
  // Wait for internal reference to wake up. See page 15, section Electrical Characteristicsm in the datasheet,
  delayMicroseconds(150);
  
  // Select test signal from chip
  // As example, this 2 methods will keep the SPI open for ADS129x chip for faster configuration. The difference It's not noticeable for humans
  // Be careful when you use this option. Read the documentation before using it.
  adsSensor.writeRegister(ads::registers::config2::REG_ADDR, ads::registers::config2::TEST_SOURCE_INTERNAL, true);
  // We will use the square signal at 4 Hz
  adsSensor.writeRegister(ads::registers::config2::REG_ADDR, ads::registers::config2::TEST_FREQ_4HZ, true);

  Serial.println("Starting channels configuration");
  Serial.println("Channel 1: gain 1 and test signal as input");
  adsSensor.enableChannelAndSetGain(1, ads::registers::chnSet::GAIN_1X, ads::registers::chnSet::TEST_SIGNAL);
  Serial.println("Channel 2: gain 3 and test signal as input");
  adsSensor.enableChannelAndSetGain(2, ads::registers::chnSet::GAIN_3X, ads::registers::chnSet::TEST_SIGNAL);
  Serial.println("Channel 3: power-down and its inputs shorted (as Texas Instruments recommends)");
  adsSensor.disableChannel(3, true);
  Serial.println("Channel 4 to max channels: set gain 6 and normal input");
  for (uint8_t i = 4; i <= ADS_N_CHANNELS; i++)
    adsSensor.enableChannelAndSetGain(i, ads::registers::chnSet::GAIN_6X, ads::registers::chnSet::ELECTRODE_INPUT);

  Serial.println("Starting channels configuration");
  adsSensor.sendSPICommandSTART();

  // We need to put ADS in DATA or RDATC mode to receive new data
  // Remember that in RDATAC mode, ADS ignores any SPI command sent if it is not SDATAC command 
  Serial.println("Set ADS chip in read data (RDATA) mode");
  adsSensor.sendSPICommandRDATA();  

  // You can could the method end() to free GPIO used pins and resources. if you don't need any more de ADS
  // You have to call begin() if you want to use again the ADS
  // adsSensor.end()
}

void loop() {
  Serial.println("New loop iteration");

  if (adsSensor.hasNewDataAvailable()) {
    Serial.println("New data available");
    ads_data_t *adsData = adsSensor.getData();
    //byte[3] statusWord = adsData->formatedData.statusWord;
    //byte[3] channel1 = adsData->formatedData.channel[0];

    Serial.println("Status word (in binary):");
    printBits(adsData->formatedData.statusWord[0]);
    Serial.print("\t");
    printBits(adsData->formatedData.statusWord[1]);
    Serial.print("\t");
    printBits(adsData->formatedData.statusWord[2]);
    Serial.println("");

    Serial.println("Channel 1 sample:");
    Serial.print(adsData->formatedData.channel[0].hi);
    Serial.print("\t");
    Serial.print(adsData->formatedData.channel[0].mid);
    Serial.print("\t");
    Serial.print(adsData->formatedData.channel[0].low);
    Serial.print("\t");

    // Transform sample to voltage
    // Remember that is in MSB (most significant bit) order!!! Position 0
    // ADS send samples in 24 bits and in binary twos complement format.
    byte *byteSample = (byte*) &adsData->formatedData.channel[0];
    // For an easy manipulation, transform twos complement format to binary offset. It is equivalent to an unsigned value
    // with V_Ref offset. The new zero is equivalent in the real world to maximum negative value possible
    byte mask = byteSample[0] & 0x80;
    mask ^= 0x80;
    byte offsetBinary = mask | (byteSample[0] & 0x7F);
    // Sample in binary offset of 24 bits. Remember, it has in MSB format. We can access with adsData->formatedData.channel[0].{hi,mid,low} fields
    int32_t sampleValue = (offsetBinary << 16) + (byteSample[1] << 8) + byteSample[2];
    // Remove offset and scale the variable before printing in the monitor
    float v_ref = 2.4; // V_ref for the ADS1294. In my setup, VREFP = 2.4V (see VREF_4V bit in config3) and VREFN = 0V (connected to ground)
    float channelGain = 1;
    float sampleInVoltsAndWithOffset = v_ref * ((float) sampleValue) / (pow(2, 23) - 1) / channelGain;
    float sampleInVolts = sampleInVoltsAndWithOffset - v_ref;
    Serial.print("Equivalent value in milivolts");
    Serial.print("\t");
    Serial.println(sampleInVolts * 1e3, 5);

    /*
        // Print in binary
        Serial.print("Original bits for channel 1: ");
        printBits(adsData->formatedData.channel[0].hi);
        printBits(adsData->formatedData.channel[0].mid);
        printBits(adsData->formatedData.channel[0].low);
        Serial.println("");
        Serial.print("Bits in offset binary for channel 1: ");
        Serial.println(sampleValue, BIN);
    */

    // Read new data. We aren't in RDATAC mode
    adsSensor.sendSPICommandRDATA();
  } else {
    Serial.println("No new data");
  }

  Serial.println("");
  
  Serial.println("Wait 1 second before execute again the loop");
  delay(1000);
}


