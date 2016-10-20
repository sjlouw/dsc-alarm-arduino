/*

DSC.h (
  Sketch to decode the keybus protocol on DSC PowerSeries 1816, 1832 and 1864 panels
   -- Use the schematic at https://github.com/emcniece/Arduino-Keybus to connect the
      keybus lines to the arduino via voltage divider circuits.  Don't forget to 
      connect the Keybus Ground to Arduino Ground (not depicted on the circuit)! You
      can also power your arduino from the keybus (+12 VDC, positive), depending on the 
      the type arduino board you have.
   -- The initial code from the github website, while claiming to use the CRC32 Library
      did not seem to reference that library.  This sketch [may] use it, so be sure to 
      download the CRC32 library from the github above and include it in your sketch
      (Though up till this point [29 Sep 16] I am not sure what it does...

  Versions:
    1.0 - 3 Sep 16 
      - Original sketch obtained from: https:github.com/emcniece/Arduino-Keybus
      - Additional code segments added from: http:www.avrfreaks.net/forum/dsc-keybus-protocol
      - The arduino will process the lines of binary data from the DSC panel on the
        keybus, while listening for connections via an Arduino ethernet shield
      - The connection acts as a TCP "Serial Monitor", streaming the output over TCP, and 
        can be accessed by navigating to the IP address in the initialization message on the 
        serial monitor, followed by /STREAM.  (Example:  http:192.168.1.45/STREAM)
    1.1 - Added ability to read Keypad messages from the data line on the downward flank of 
        the clock signal. (May still need work on timing, but data seems good)
    1.2 - Changed timing and logic involved with reading and processing words from the keypad
        and panel to make it more reliable
    1.3 - 
  
  Created by stagf15, 19 Jul 2016.
  Released into the public domain.
  
*/

#ifndef DSC_h
#define DSC_h
#include "DSC_Globals.h"
#include "DSC_Constants.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class DSC : public Print  // Initialize DSC as an extension of the print class
{
  public:
    // Class to call to initialize the DSC Class
    // for example...  DSC dsc;
    DSC(void);
    
    // Used to add the serial instance to the DSC Class
    int addSerial(void);
    
    // Included in the setup function of the user's sketch
    // Begins the the class, sets the pin modes, attaches the interrupt
    void begin(void);
    
    // Included in the main loop of user's sketch, checks and processes 
    // the current panel and keypad words if able
    // Returns:   0   (No 
    int process(void);
    
    // Decodes the panel and keypad words, returns 0 for failure and the command
    // byte for success
    byte decodePanel(void);
    byte decodeKeypad(void);
    
    // Ends, or de-constructs the class - NOT USED  
    //int end();
    
    // Prints the panel and keypad word in formatted binary (returns 0 as default)
    int pnlBinary(String &dataStr);    // Returns 1 for valid checksum
    int kpdBinary(String &dataStr);
    
    // Returns 1 if there is a valid checksum, 0 if not
    int pnlChkSum(String &dataStr);
    
    unsigned int binToInt(String &dataStr, int offset, int dataLen);
    
    String byteToBin(byte b);
   
    void zeroArr(byte byteArr[]);
    void setCLK(int p);
    void setDTA_IN(int p);
    void setDTA_OUT(int p);
    void setLED(int p);
   
    // ----- Print class extension variables -----
    virtual size_t write(uint8_t);
    virtual size_t write(const char *str);
    virtual size_t write(const uint8_t *buffer, size_t size);

  private:
    uint8_t intrNum;
};

#endif