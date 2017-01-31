# ADS129x-driver
This is a driver for Arduino boards to control the ADS 129x chips family of Texas Instrument

The complete documentation is in ads129xDriver.h file but a quick introduction is:
* ads129xDriver.h -> it has the documentation and the methods
* ads129xDatasheetConstants.h -> it contains the constant defined by datasheet and some other useful constant to configure the registers
* ads129xDriverConfig.h -> the only file to be modified by user. In this, user have to speficy ADS model that they will use and, optionally, some other parameters.

The license of this library is Mozilla Public License version 2 (see license notice) (https://www.mozilla.org/en-US/MPL/). From the Mozilla Public License (MPL) FAQs: the MPL is a simple copyleft license. The MPL's "file-level" copyleft is designed to encourage contributors to share modifications they make to your code, while still allowing them to combine your code with code under other licenses (open or proprietary) with minimal restrictions.
 
So,
  1. Your own code can have any license except see point 2
  2. If you modify any file/s of this library, the resulting file/s of these modifications have to keep the Mozilla Public License version 2 license.
  3. If you distribute/sell a program that some files use Mozilla Public License version 2 license (like this library), the source code of these files have to share 
  4. MPL is compatible with GPL license family.
  
## Credits    
* This library is inspired by ADS129x-tools (https://github.com/adamfeuer/ADS129x-tools)
* Antonio and Marc for the support given during the development
