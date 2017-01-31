#include "ads129xDriver.h"

#include <Arduino.h>
#include <SPI.h>

#if ADS_LIBRARY_VERBOSE_LEVEL > 0
// Code are wrapped in {} because must be treated as one line of code
#define _ADS_WARNING(msg) \
  {Serial.print("Warning: "); \
    Serial.println(msg); \
    Serial.println("Stopping program execution"); \
    while (1); }
#else
#define _ADS_WARNING(msg) ; // Do nothing
#endif

#if ADS_LIBRARY_VERBOSE_LEVEL > 0
// Code are wrapped in {} because must be treated as one line of code
#define _ADS_ERROR(msg) \
  {Serial.print("Error: "); \
    Serial.println(msg); \
    Serial.println("Stopping program execution"); \
    while (1); }
#else
#define _ADS_ERROR(msg) while(1);
#endif

/* ======= Wrapper to workaround the attachInterrupt limitation  ============= */
// Attach interrupt doesn't work with methods in class. Only with global functions or static methods.
// To workaround and to avoid declare multitud methods of ADS129xSensor as statics, I keep a instance
// of the class in a global variable and call the right function from a global function
ADS129xSensor *_ADS129xSensorPrivateInstance_ = 0; // Pointer to ADS129xx instance. It is used to call

void _ISR_ADS_privateReadDataFromChip_() {
  _ADS129xSensorPrivateInstance_->_privateReadDataFromChip_();
}

void ADS129xSensor::begin() {
  // Add this class to the instance
  if (_ADS129xSensorPrivateInstance_ == 0)
    _ADS129xSensorPrivateInstance_ = this;
  else {
    _ADS_ERROR("The library allows only one ADS129xSensor object to be inicialized. You must call end() method in the other ADS129xSensor");
  }

  using namespace ads::registers;

  // start the SPI library:
  SPI.begin();

  // Mandatory pins configuration
  // Chip select pin configuration
  pinMode(this->chipSelectPin, OUTPUT);
  digitalWrite(this->chipSelectPin, HIGH);

  // DRDY (data ready) pin configuration
  pinMode(drdyPin, INPUT);
  // DRDY pin used to interrupt is attached to the Arduino
  attachInterrupt(digitalPinToInterrupt(drdyPin), _ISR_ADS_privateReadDataFromChip_, FALLING);
  SPI.usingInterrupt(digitalPinToInterrupt(drdyPin)); // Disable the interrupt when there is a SPI transaction in course

  // ADS configuration
  // See page 85 in the datashhet for more information about ADS129XX boot up sequency.
  // See page page 84, 10.1.1 Setting the Device for Basic Data Capture, in the datasheet
  // Starting power-up sequency
#if ADS_LIBRARY_VERBOSE_LEVEL > 0
  Serial.println("Starting power-up sequency");
#endif
  // We wait _ADS_POWER_UP_DELAY_MS miliseconds before sent any command to ADS12XX
  // Also, we Wait for t_por and VCAP1 > 1.1V
  // Moreover, give to to the internal oscillator to start up (it is 20 microseconds (see electrical characteristics in the datasheet)
  delay(_ADS_POWER_UP_DELAY_MS);

  // Set CLKSEL, START and PDWN pins to default value if are provided by user specified in
  // page 84, 10.1.1 Setting the Device for Basic Data Capture, in the datasheet
  // I also configure the arduino pins connected to them

  // If clksel pin is specified, external clock is provided to ADS chip.
  if (clkselPin != ADS_PIN_NOT_USED) {
    pinMode(this->clkselPin, OUTPUT);
    enableExternalClockSource();
  }

  // Start pin
  if (startPin != ADS_PIN_NOT_USED) {
    pinMode(this->startPin, OUTPUT);
    disableHardwareStartMode();
  }

  // Pdwn Pin
  if (pwdnPin != ADS_PIN_NOT_USED) {
    pinMode(this->pwdnPin, OUTPUT);
    disableHardwarePowerDownMode();
  }

  // Reset pin
  if (resetPin != ADS_PIN_NOT_USED) {
    pinMode(this->resetPin, OUTPUT);
    digitalWrite(this->resetPin, HIGH);
  }

  // Reset ADS by hardware or software reset. It is indifferent. Hardware reset is the prefered method
  resetADS();
  // Stop RDATAC mode (ads129x restart by default in this configuration). See page 62, section 9.5.2.6 RDATAC: Read Data Continuous, in the datasheet
  sendSPICommandSDATAC(false);

  // Checking that ADS is the right model
  byte idRegister = readRegister(id::REG_ADDR);
  uint8_t correctIdChip = 0;
  switch (ADS_CHIP_USED) {
    case ADS_1294: correctIdChip = id::ID_ADS1294;   break;
    case ADS_1294R: correctIdChip = id::ID_ADS1294R;   break;
    case ADS_1296: correctIdChip = id::ID_ADS1296;   break;
    case ADS_1296R: correctIdChip = id::ID_ADS1296R;   break;
    case ADS_1298: correctIdChip = id::ID_ADS1298;   break;
    case ADS_1298R: correctIdChip = id::ID_ADS1298R;   break;
  }

  if (idRegister != correctIdChip)
    _ADS_ERROR("ID reported from ADS chip and the chip model configurated by user are not the same => Theorical y real ADS models are not the same !!!!!");

  // Power up sequency completed
#if ADS_LIBRARY_VERBOSE_LEVEL > 0
  Serial.println("Power-up sequency completed");
#endif
}

void ADS129xSensor::end() {
  sendSPICommandSDATAC(true);
  sendSPICommandSTOP(true);
  _ADS129xSensorPrivateInstance_ = NULL;
}

void ADS129xSensor::resetADS() {
  // FIXME: en estos comentarios, la construcción con "prefer" es correcta??
  // Hardware reset is prefered over software reset.
  if (resetPin == ADS_PIN_NOT_USED) {
    // Software reset is performed
    // If ADS129X is in read data continuous mode (RDATAC), SDATAC command must be issued before any other
    // commands can be sent to the device. If ADS129X is not in RDATAC mode, this command is ignore by chip.
    // See page 63, section 9.5.2.7 SDATAC: Stop Read Data Continuous, in the datasheet

    // Note: I am aware that readingStatus attribute reflect the status of the ADS129X with respect to read mode-
    //       However, if I used it and in some piece of code has a bug, chip could be not reseted. So, I prefer to
    //       make sure that chip ALWAYS is reseted.
    sendSPICommandSDATAC(true);
    sendSPICommandRESET(true);
  } else {
    // Hardware reset is performed
    // No need for more worries ;)
    doHardwareReset();
  }
  // Remember that ADS12XX enters in read data continuous mode (RDATAC) after reset command. See page 62, section 9.5.2.6 RDATAC: Read Data Continuous, in the datasheet
  readingStatus = _ADS_READING_DATA_IN_RDATAC_MODE;
}

// Interruption won't be called if SPI is in use
void ADS129xSensor::_privateReadDataFromChip_() {
  if (readingStatus == _ADS_NO_READING_NEW_DATA)
    return; // It is not needed to read the new available data
  else if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE)
    beginSpiTransaction();
  else {
    sendCommand(ads::commands::RDATA, true); // Leave open SPI transaction
    // Only one sample need to be read -> later sample must be ignored
    readingStatus = _ADS_NO_READING_NEW_DATA;
  }

  // Cleaning the SPI buffer
  for (uint8_t i = 0; i < _ADS_DATA_PACKAGE_SIZE; i++)
    adsData.rawData[i] = 0x00;

  void *buffer = (void*) &adsData.rawData;
  SPI.transfer(buffer, _ADS_DATA_PACKAGE_SIZE);

  hasNewData = true;
  endSpiTransaction();
}

/* ====== Methods that use hardware pins ========== */
void ADS129xSensor::doHardwareReset() {
  if (resetPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("Reset pin is not specified!!!");

  digitalWrite(this->resetPin, LOW);
  delay(_ADS_T_CLK_2);
  digitalWrite(this->resetPin, HIGH);
}

void ADS129xSensor::enableHardwareStartMode() {
  if (startPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("Start pin is not specified!!!");

  // See page 51, section 9.4.1.1 Start mode, in the datasheet for more information
  digitalWrite(startPin, HIGH);
  delay(_ADS_T_CLK_2);
}

void ADS129xSensor::disableHardwareStartMode() {
  if (startPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("Start pin is not specified!!!");

  // See page 51, section 9.4.1.1 Start mode, in the datasheet for more information
  digitalWrite(startPin, LOW);
  delay(_ADS_T_CLK_2);
}

void ADS129xSensor::enableExternalClockSource() {
  if (clkselPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("Clksel pin is not specified!!!");

  // See page 5, section 9.3.2.5 Clock, in the datasheet for more information
  digitalWrite(clkselPin, LOW);
}

void ADS129xSensor::disableExternalClockSource() {
  if (clkselPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("Clksel pin is not specified!!!");

  // See page 5, section 9.3.2.5 Clock, in the datasheet for more information
  digitalWrite(clkselPin, HIGH);
  // Wait for internal clock power up. See page 15, Elecrical Characteristics, in the datasheet
  delayMicroseconds(20);
}

// Only it can be used if pwdn pin is specified
void ADS129xSensor::enableHardwarePowerDownMode() {
  if (pwdnPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("PWDN pin is not specified!!!");

  // See page 48, section 9.3.2.2 Power-Down Pin (PWDN) in the datasheet for more details about powerup after a power down.
  digitalWrite(this->pwdnPin, LOW);
}
// Only it can be used if pwdn pin is specified
void ADS129xSensor::disableHardwarePowerDownMode() {
  if (pwdnPin == ADS_PIN_NOT_USED)
    _ADS_ERROR("PWDN pin is not specified!!!");

  // See page 48, section 9.3.2.2 Power-Down Pin (PWDN) in the datasheet for more details about powerup after a power down.
  digitalWrite(this->pwdnPin, HIGH);
  // Upon exiting from power-down mode, the internal oscillator and the reference require time to wakeup.
  // Wake up time for the oscillator is 20 microseconds. See page 15, section Electrical Characteristicsm - Clock, in the datasheet
  // Wake up time for the internal reference is 150 microseconds. See page 15, section Electrical Characteristicsm - Internal Reference, in the datasheet
  delayMicroseconds(150); // I assume the worst case escenario -> wait for internal reference.
}

// See page 17, section 7.7 Switching Characteristics: Serial Interface, and page 59, section 9.5 Programming, in the datasheet) to understand SPI communication
void ADS129xSensor::beginSpiTransaction() {
  // Configure SPI communication
  if (!this->isSpiOpen) {
    // Delays aren0't need because Arduino is slow enough to execute beginTransaction and endTransaction functions
    SPI.beginTransaction(SPISettings(_ADS_SPI_MAX_SPEED, _ADS_SPI_BIT_ORDER, _ADS_SPI_MODE));
    digitalWrite(this->chipSelectPin, LOW);
    // delayMicroseconds(_ADS_T_CSSC);
    this->isSpiOpen = true;
  } // It is already opened !!!
}

// See page 17, section 7.7 Switching Characteristics: Serial Interface, and page 59, section 9.5 Programming, in the datasheet) to understand SPI communication
void ADS129xSensor::endSpiTransaction() {
  if (this->isSpiOpen) {
    isSpiOpen = false;
    // Delays aren0't need because Arduino is slow enough to execute beginTransaction and endTransaction functions
    // delayMicroseconds(_ADS_T_SCCS);
    digitalWrite(this->chipSelectPin, HIGH);
    // delayMicroseconds(_ADS_T_CSH);

    SPI.endTransaction();
  }
}


/* ============ Registers ============== */
// See page 17, section 7.7 Switching Characteristics: Serial Interface, and page 59, section 9.5 Programming, in the datasheet) to understand SPI communication
byte ADS129xSensor::readRegister(byte registAddr, boolean keepSpiOpen) {
  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands (like read/write registers) will be ignored "); 
    return 0xFF;
  }

  // In datasheet is not specified but I think that chip select must be low for the entire command.
  // because in case of writting in a register, chip select must be low in the entire operation
  beginSpiTransaction();

  // Send write regiter command y the first register that will be written
  SPI.transfer(ads::commands::RREG | registAddr);
  // Send the number the register that will be written minus 1. Ex: 1 register will be written -> 0
  SPI.transfer(0x00);
  // DIN must be LOW when data is read
  byte registerValue = SPI.transfer(0x00);

  if (!keepSpiOpen)
    endSpiTransaction();

  return registerValue;
}

// See page 17, section 7.7 Switching Characteristics: Serial Interface, and page 59, section 9.5 Programming, in the datasheet) to understand SPI communication
void ADS129xSensor::writeRegister(byte registAddr, byte data, boolean keepSpiOpen) {
  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands (like read/write registers) will be ignored "); 
    return;
  }

  // Chip select must be low for the entire command
  beginSpiTransaction();

  // Send write regiter command y the first register that will be written
  SPI.transfer(ads::commands::WREG | registAddr);
  // Send the number the register that will be written minus 1. Ex: 1 register will be written -> 0x00
  SPI.transfer(0x00);
  // Write register
  SPI.transfer(data);

  // When resp or config1 registers are written, internal reset is performed. See page 48, section 9.3.2.3 Reset (RESET Pin and Reset Command), in the datasheet
  using namespace ads::registers;
  if (registAddr == config1::REG_ADDR || registAddr == resp::REG_ADDR)
    delay(_ADS_T_CLK_18);

  if (!keepSpiOpen)
    endSpiTransaction();
}

void ADS129xSensor::setAllRegisterToResetValuesWithoutResetCommand( boolean keepSpiOpen) {
  using namespace ads::registers;

  // ID register is read only
  writeRegister(config1::REG_ADDR, config1::RESET_VALUE | config1::RESERVED_BITS, true);
  writeRegister(config2::REG_ADDR, config2::RESET_VALUE | config2::RESERVED_BITS, true);
  writeRegister(config3::REG_ADDR, config3::RESET_VALUE | config3::RESERVED_BITS, true);
  writeRegister(loff::REG_ADDR, loff::RESET_VALUE | loff::RESERVED_BITS, true);

  // Channels registers
  for (uint8_t i = 0; i < ADS_N_CHANNELS; i++)
    writeRegister(chnSet::_BASE_REG_ADDR + i, chnSet::RESET_VALUE | chnSet::RESERVED_BITS, true);

  writeRegister(rldSensp::REG_ADDR, rldSensp::RESET_VALUE | rldSensp::RESERVED_BITS, true);
  writeRegister(rldSensn::REG_ADDR, rldSensn::RESET_VALUE | rldSensn::RESERVED_BITS, true);
  writeRegister(loffSensp::REG_ADDR, loffSensp::RESET_VALUE | loffSensp::RESERVED_BITS, true);
  writeRegister(loffSensn::REG_ADDR, loffSensn::RESET_VALUE | loffSensn::RESERVED_BITS, true);
  writeRegister(loffFlip::REG_ADDR, loffFlip::RESET_VALUE | loffFlip::RESERVED_BITS, true);

  // loff_statp and loff_statn are read-only registers
  writeRegister(gpio::REG_ADDR, gpio::RESET_VALUE | gpio::RESERVED_BITS, true);
  writeRegister(pace::REG_ADDR, pace::RESET_VALUE | pace::RESERVED_BITS, true);
  writeRegister(resp::REG_ADDR, resp::RESET_VALUE | resp::RESERVED_BITS, true);
  writeRegister(config4::REG_ADDR, config4::RESET_VALUE | config4::RESERVED_BITS, true);
  writeRegister(wct1::REG_ADDR, wct1::RESET_VALUE | wct1::RESERVED_BITS, true);
  writeRegister(wct2::REG_ADDR, wct2::RESET_VALUE | wct2::RESERVED_BITS, keepSpiOpen);
}

/* ============ Commands ============== */
// See page 17, section 7.7 Switching Characteristics: Serial Interface, and page 59, section 9.5 Programming, in the datasheet) to understand SPI communication
void ADS129xSensor::sendCommand(byte command, boolean keepSpiOpen) {
  beginSpiTransaction();

  // Send command
  byte aux = SPI.transfer(command);

#if ADS_LIBRARY_VERBOSE_LEVEL > 1
  Serial.print("Command sent: ");
  Serial.println(command, BIN);
#endif

  if (!keepSpiOpen)
    endSpiTransaction();
}

void ADS129xSensor::sendSPICommandWAKEUP(boolean keepSpiOpen) {
  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored"); 
    return;
  }

  //After execute WAKEUP command, the next command must wait for 4*_ADS_T_CLK cycles (see page 61 in the datasheet)
  // 4*_ADS_T_CLK is roughtly 2 microseconds
  sendCommand(ads::commands::WAKEUP, keepSpiOpen);
  delayMicroseconds(_ADS_T_CLK_4);
}

void ADS129xSensor::sendSPICommandSTANDBY(boolean keepSpiOpen) {
  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored"); 
    return;
  }

  sendCommand(ads::commands::STANDBY, keepSpiOpen);
  // No wait time needed
}

void ADS129xSensor::sendSPICommandRESET(boolean keepSpiOpen) {
  if (resetPin != ADS_PIN_NOT_USED)
    _ADS_ERROR("Reset pin is specified!!!! Reset must be done with the reset pin -> use doHardwareReset() method!!!");

  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored"); 
    return;
  }

  // 18*_ADS_T_CLK cycles are required to execute the RESET command. Do not send any commands during this time. (see page 62 in datasheet)
  // 18 *_ADS_T_CLK is roughtly 8.8 microseconds
  sendCommand(ads::commands::RESET, keepSpiOpen);
  delayMicroseconds(_ADS_T_CLK_18);
}

void ADS129xSensor::sendSPICommandSTART(boolean keepSpiOpen) {
  if (startPin != ADS_PIN_NOT_USED)
    _ADS_ERROR("Start pin is specified -> start and stop are not allowed!!!");

  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored"); 
    return;
  }

  //After execute START command, the next command must wait for 4*_ADS_T_CLK cycles (see page 62 in the datasheet)
  // 4*_ADS_T_CLK is roughtly 2 microseconds
  sendCommand(ads::commands::START, keepSpiOpen);
  delayMicroseconds(_ADS_T_CLK_4); // Only is necesarry if just after is sent STOP command
}

void ADS129xSensor::sendSPICommandSTOP(boolean keepSpiOpen) {
  if (startPin != ADS_PIN_NOT_USED)
    _ADS_ERROR("Start pin is specified -> start and stop are not allowed!!!");

  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE){
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored"); 
    return;
  }

  sendCommand(ads::commands::STOP, keepSpiOpen);
  // No wait time needed
}

// Be aware that when RDATAC command is sent, the any other command except SDATAC will be ignored
void ADS129xSensor::sendSPICommandRDATAC(boolean keepSpiOpen) {
  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE) {
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored");
    return;
  }

  //After execute RDATAC command, the next command must wait for 4*_ADS_T_CLK cycles (see page 62 in the datasheet)
  // 4*_ADS_T_CLK is roughtly 2 microseconds
  sendCommand(ads::commands::RDATAC, keepSpiOpen);
  delayMicroseconds(_ADS_T_CLK_4);
  readingStatus = _ADS_READING_DATA_IN_RDATAC_MODE;
}

void ADS129xSensor::sendSPICommandSDATAC(boolean keepSpiOpen) {
  //Afterfreturn execute SDATAC command, the next command must wait for 4*_ADS_T_CLK cycles (see page 63 in the datasheet)
  // 4*_ADS_T_CLK is roughtly 2 microseconds
  sendCommand(ads::commands::SDATAC, keepSpiOpen);
  delayMicroseconds(_ADS_T_CLK_4);
  readingStatus = _ADS_NO_READING_NEW_DATA;
}

void ADS129xSensor::sendSPICommandRDATA(boolean keepSpiOpen) {
  if (readingStatus == _ADS_READING_DATA_IN_RDATAC_MODE) {
    _ADS_WARNING("In RDATAC mode, ADS only accept SDATAC SPI command. Others commands will be ignored");
    return;
  }

  // RDATA command will be send when new data is available
  readingStatus = _ADS_READING_DATA_IN_RDATA_MODE;
#if ADS_LIBRARY_VERBOSE_LEVEL > 1
  Serial.print("readingStatus: ");
  Serial.println(readingStatus);
#endif
}

/* ======= Class methods class implementing typical ADS configurations  ============= */
void ADS129xSensor::disableChannel(uint8_t nChannel, boolean setInputAsShorted, boolean keepSpiOpen) {
  using namespace ads::registers::chnSet;
  byte registerAddress = _BASE_REG_ADDR + nChannel;
  byte registerValue = readRegister(registerAddress, true);
  // Power down the channel --> write 1 in bit 7
  writeRegister(registerAddress, registerValue | B_PDn, keepSpiOpen);
}

void ADS129xSensor::enableChannel(uint8_t nChannel, int8_t channelInput, boolean keepSpiOpen) {
  if (nChannel == 0)
    _ADS_ERROR("nChannel is zero");

  if (nChannel > ADS_N_CHANNELS)
    _ADS_ERROR("nChannel is bigger than the number of channels that has the chip ADS");

  using namespace ads::registers::chnSet;
  byte registerAddress = _BASE_REG_ADDR + nChannel;
  byte registerValue = readRegister(registerAddress, true);
  if (channelInput != -1) {
    byte mask = B_MUXn2 | B_MUXn1 | B_MUXn0;
    registerValue = registerValue & ~mask; // Remove current channel input configuration
    registerValue = registerValue | (channelInput & mask); // Set the new channel input configuration
  }
  // Power up the channel --> write 0 in bit _PDn
  writeRegister(registerAddress, registerValue & (~B_PDn), keepSpiOpen);
}

void ADS129xSensor::enableChannelAndSetGain(uint8_t nChannel, byte channelGainConstant, int8_t channelInput, boolean keepSpiOpen) {
  using namespace ads::registers::chnSet;
  byte registerAddress = _BASE_REG_ADDR + nChannel;
  byte registerValue = readRegister(registerAddress, true);
  // Set 0 the gain bits
  registerValue = registerValue & (~(B_GAINn0 | B_GAINn1 | B_GAINn2));
  // Set the channel gain
  registerValue = registerValue | channelGainConstant;
  writeRegister(registerAddress, registerValue, true);
  // Power up the channel
  enableChannel(nChannel, channelInput, keepSpiOpen);
}


