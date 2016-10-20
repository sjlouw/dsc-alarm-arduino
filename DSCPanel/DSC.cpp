#include "Arduino.h"
#include "DSC.h"
#include "DSC_Constants.h"
#include "DSC_Globals.h"
#include <TimeLib.h>

/*
 * The existence of global variables are "declared" in DSC_Global.h, so that each 
 * source file that includes the header knows about them. The variables must then be
 * “defined” once in one of the source files (this one).

 * The following structure contains all of the global variables used by the ISR to 
 * communicate with the DSC.clkCalled() object. You cannot pass parameters to an 
 * ISR so these values must be global. The fields are defined in DSC_Globals.h
 */
dscGlobal_t dscGlobal;

// ----- Input/Output Pins (Global) -----
byte CLK;         // Keybus Yellow (Clock Line)
byte DTA_IN;      // Keybus Green (Data Line via V divider)
byte DTA_OUT;     // Keybus Green Output (Data Line through driver)
byte LED;         // LED pin on the arduino

void clkCalled_Handler(); // Prototype for interrupt handler, called on clock line change

DSC::DSC(void)
  {
    // ----- Time Variables -----
    // Volatile variables, modified within ISR
    dscGlobal.intervalTimer = 0;   
    dscGlobal.clockChange = 0;
    dscGlobal.lastChange = 0;      
    dscGlobal.lastRise = 0;        // NOT USED
    dscGlobal.lastFall = 0;        // NOT USED
    dscGlobal.newWord = false;     // NOT USED

    // ------ PUBLIC VARIABLES ------

    // ----- Input/Output Pins (DEFAULTS) ------
    //   These can be changed prior to DSC.begin() using functions below
    CLK      = 3;    // Keybus Yellow (Clock Line)
    DTA_IN   = 4;    // Keybus Green (Data Line via V divider)
    DTA_OUT  = 8;    // Keybus Green Output (Data Line through driver)
    LED      = 13;   // LED pin on the arduino

    // ----- Time Variables -----
    dscGlobal.lastStatus = 0;
    dscGlobal.lastData = 0;

    // ----- Keybus Word String Vars -----
    dscGlobal.pBuild="", dscGlobal.pWord="";
    dscGlobal.oldPWord="", dscGlobal.pMsg="";
    dscGlobal.kBuild="", dscGlobal.kWord="";
    dscGlobal.oldKWord="", dscGlobal.kMsg="";
    dscGlobal.pCmd, dscGlobal.kCmd = 0;

    // ----- Byte Array Variables -----
    //dscGlobal.pBytes[ARR_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};    // NOT USED
    //dscGlobal.kBytes[ARR_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};    // NOT USED
  }

int DSC::addSerial(void)
  {
  }

void DSC::begin(void)
  {
    pinMode(CLK, INPUT);
    pinMode(DTA_IN, INPUT);
    pinMode(DTA_OUT, OUTPUT);
    pinMode(LED, OUTPUT);

    // Set the interrupt pin
    intrNum = digitalPinToInterrupt(CLK);

    // Attach interrupt on the CLK pin
    attachInterrupt(intrNum, clkCalled_Handler, CHANGE);  
    //   Changed from RISING to CHANGE to read both panel and keypad data
  }

int DSC::process(void)
  {
    // ----------------- Turn on/off LED ------------------
    if ((millis() - dscGlobal.lastStatus) > 500)
      digitalWrite(LED, 0);     // Turn LED OFF (no recent status command [0x05])
    else
      digitalWrite(LED, 1);     // Turn LED ON  (recent status command [0x05])
    
    // ---------------- Get/process incoming data ----------------
    //dscGlobal.pCmd, dscGlobal.kCmd = 0;  // THIS PROBABLY SHOULDN'T BE HERE!!!
    
    /*
     * The normal clock frequency is 1 Hz or one cycle every ms (1000 us) 
     * The new word marker is clock high for about 15 ms (15000 us)
     * If the interval is longer than the required amount (NEW_WORD_INTV), 
     * and the panel word in progress (pBuild) is more than 8 characters long,
     * process the panel and keypad words, otherwise return failure (0).
     */
    if ((dscGlobal.intervalTimer < (NEW_WORD_INTV + 200)) || 
            (dscGlobal.pBuild.length() < 8)) return 0;  // Return failure

    dscGlobal.pWord = dscGlobal.pBuild;   // Save the complete panel raw data bytes sentence
    dscGlobal.pBuild = "";                // Reset the raw data panel word being built
    dscGlobal.pMsg = "";                  // Initialize panel message for output
    dscGlobal.kMsg = "";                  // Initialize keypad message for output
    dscGlobal.pCmd, dscGlobal.kCmd = 0;
    
    dscGlobal.pCmd = decodePanel();       // Decode the panel binary, return command byte, or 0
    dscGlobal.kCmd = decodeKeypad();      // Decode the keypad binary, return command byte, or 0
    
    if (dscGlobal.pCmd && dscGlobal.kCmd) return 3;  // Return 3 if both were decoded
    else if (dscGlobal.kCmd) return 2;    // Return 2 if keypad word was decoded
    else if (dscGlobal.pCmd) return 1;    // Return 1 if panel word was decoded
    else return 0;                        // Return failure if none were decoded
  }

byte DSC::decodePanel(void) 
  {
    // ------------- Process the Panel Data Word ---------------
    byte cmd = binToInt(dscGlobal.pWord,0,8);       // Get the panel pCmd (data word type/command)
    
    if (dscGlobal.pWord == dscGlobal.oldPWord || cmd == 0x00) {
      // Skip this word if the data hasn't changed, or pCmd is empty (0x00)
      return 0;     // Return failure
    }
    else {     
      // This seems to be a valid word, try to process it  
      dscGlobal.lastData = millis();                // Record the time (last data word was received)
      dscGlobal.oldPWord = dscGlobal.pWord;                   // This is a new/good word, save it
     
      // Interpret the data
      if (cmd == 0x05) 
      {
        dscGlobal.lastStatus = millis();            // Record the time for LED logic
        dscGlobal.pMsg += F("[Status] ");
        if (binToInt(dscGlobal.pWord,16,1)) {
          dscGlobal.pMsg += F("Ready");
        }
        else {
          dscGlobal.pMsg += F("Not Ready");
        }
        if (binToInt(dscGlobal.pWord,12,1)) dscGlobal.pMsg += F(", Error");
        if (binToInt(dscGlobal.pWord,13,1)) dscGlobal.pMsg += F(", Bypass");
        if (binToInt(dscGlobal.pWord,14,1)) dscGlobal.pMsg += F(", Memory");
        if (binToInt(dscGlobal.pWord,15,1)) dscGlobal.pMsg += F(", Armed");
        if (binToInt(dscGlobal.pWord,17,1)) dscGlobal.pMsg += F(", Program");
        if (binToInt(dscGlobal.pWord,29,1)) dscGlobal.pMsg += F(", Power Fail");   // ??? - maybe 28 or 20?
      }
     
      if (cmd == 0xa5)
      {
        dscGlobal.pMsg += F("[Info] ");
        int y3 = binToInt(dscGlobal.pWord,9,4);
        int y4 = binToInt(dscGlobal.pWord,13,4);
        int yy = (String(y3) + String(y4)).toInt();
        int mm = binToInt(dscGlobal.pWord,19,4);
        int dd = binToInt(dscGlobal.pWord,23,5);
        int HH = binToInt(dscGlobal.pWord,28,5);
        int MM = binToInt(dscGlobal.pWord,33,6);
        
        setTime(HH,MM,0,dd,mm,yy);
        if (timeStatus() == timeSet) {
          Serial.println(F("Time Synchronized"));
          dscGlobal.pMsg += F("Time Sync ");
        }
        else {
          Serial.println(F("Time Sync Error"));
          dscGlobal.pMsg += F("Time Sync Error ");
        }      

        byte arm = binToInt(dscGlobal.pWord,41,2);
        byte master = binToInt(dscGlobal.pWord,43,1);
        byte user = binToInt(dscGlobal.pWord,43,6); // 0-36
        if (arm == 0x02) {
          dscGlobal.pMsg += F(", Armed");
          user = user - 0x19;
        }
        if (arm == 0x03) {
          dscGlobal.pMsg += F(", Disarmed");
        }
        if (arm > 0) {
          if (master) dscGlobal.pMsg += F(", Master Code"); 
          else dscGlobal.pMsg += F(", User Code");
          user += 1; // shift to 1-32, 33, 34
          if (user > 34) user += 5; // convert to system code 40, 41, 42
          dscGlobal.pMsg += " " + String(user);
        }
      }
      
      if (cmd == 0x27)
      {
        dscGlobal.pMsg += F("[Zones A] ");
        int zones = binToInt(dscGlobal.pWord,8+1+8+8+8+8,8);
        if (zones & 1) dscGlobal.pMsg += "1";
        if (zones & 2) dscGlobal.pMsg += "2";
        if (zones & 4) dscGlobal.pMsg += "3";
        if (zones & 8) dscGlobal.pMsg += "4";
        if (zones & 16) dscGlobal.pMsg += "5";
        if (zones & 32) dscGlobal.pMsg += "6";
        if (zones & 64) dscGlobal.pMsg += "7";
        if (zones & 128) dscGlobal.pMsg += "8";
        if (zones == 0) dscGlobal.pMsg += "Ready ";
      }
      
      if (cmd == 0x2d)
      {
        dscGlobal.pMsg += F("[Zones B] ");
        int zones = binToInt(dscGlobal.pWord,8+1+8+8+8+8,8);
        if (zones & 1) dscGlobal.pMsg += "9";
        if (zones & 2) dscGlobal.pMsg += "10";
        if (zones & 4) dscGlobal.pMsg += "11";
        if (zones & 8) dscGlobal.pMsg += "12";
        if (zones & 16) dscGlobal.pMsg += "13";
        if (zones & 32) dscGlobal.pMsg += "14";
        if (zones & 64) dscGlobal.pMsg += "15";
        if (zones & 128) dscGlobal.pMsg += "16";
        if (zones == 0) dscGlobal.pMsg += "Ready ";
      }
      
      if (cmd == 0x34)
      {
        dscGlobal.pMsg += F("[Zones C] ");
        int zones = binToInt(dscGlobal.pWord,8+1+8+8+8+8,8);
        if (zones & 1) dscGlobal.pMsg += "17";
        if (zones & 2) dscGlobal.pMsg += "18";
        if (zones & 4) dscGlobal.pMsg += "19";
        if (zones & 8) dscGlobal.pMsg += "20";
        if (zones & 16) dscGlobal.pMsg += "21";
        if (zones & 32) dscGlobal.pMsg += "22";
        if (zones & 64) dscGlobal.pMsg += "23";
        if (zones & 128) dscGlobal.pMsg += "24";
        if (zones == 0) dscGlobal.pMsg += "Ready ";
      }
      
      if (cmd == 0x3e)
      {
        dscGlobal.pMsg += F("[Zones D] ");
        int zones = binToInt(dscGlobal.pWord,8+1+8+8+8+8,8);
        if (zones & 1) dscGlobal.pMsg += "25";
        if (zones & 2) dscGlobal.pMsg += "26";
        if (zones & 4) dscGlobal.pMsg += "27";
        if (zones & 8) dscGlobal.pMsg += "28";
        if (zones & 16) dscGlobal.pMsg += "29";
        if (zones & 32) dscGlobal.pMsg += "30";
        if (zones & 64) dscGlobal.pMsg += "31";
        if (zones & 128) dscGlobal.pMsg += "32";
        if (zones == 0) dscGlobal.pMsg += "Ready ";
      }
      // --- The other 32 zones for a 1864 panel need to be added after this ---

      if (cmd == 0x11) {
        dscGlobal.pMsg += F("[Keypad Query] ");
      }
      if (cmd == 0x0a) {
        dscGlobal.pMsg += F("[Panel Program Mode] ");
      } 
      if (cmd == 0x5d) {
        dscGlobal.pMsg += F("[Alarm Memory Group 1] ");
      } 
      if (cmd == 0x63) {
        dscGlobal.pMsg += F("[Alarm Memory Group 2] ");
      } 
      if (cmd == 0x64) {
        dscGlobal.pMsg += F("[Beep Command Group 1] ");
      } 
      if (cmd == 0x69) {
        dscGlobal.pMsg += F("[Beep Command Group 2] ");
      } 
      if (cmd == 0x39) {
        dscGlobal.pMsg += F("[Undefined command from panel] ");
      } 
      if (cmd == 0xb1) {
        dscGlobal.pMsg += F("[Zone Configuration] ");
      }
    return cmd;     // Return success
    }
  }

byte DSC::decodeKeypad(void) 
  {
    // ------------- Process the Keypad Data Word ---------------
    byte cmd = binToInt(dscGlobal.kWord,0,8);     // Get the keypad pCmd (data word type/command)
    String btnStr = F("[Button] ");

    if (dscGlobal.kWord.indexOf("0") == -1) {  
      // Skip this word if kWord is all 1's
      return 0;     // Return failure
    }
    else { 
      // This seems to be a valid word, try to process it
      dscGlobal.lastData = millis();              // Record the time (last data word was received)
      dscGlobal.oldKWord = dscGlobal.kWord;                 // This is a new/good word, save it

      byte kByte2 = binToInt(dscGlobal.kWord,8,8); 
     
      // Interpret the data
      if (cmd == kOut) {
        if (kByte2 == one)
          dscGlobal.kMsg += btnStr + "1";
        else if (kByte2 == two)
          dscGlobal.kMsg += btnStr + "2";
        else if (kByte2 == three)
          dscGlobal.kMsg += btnStr + "3";
        else if (kByte2 == four)
          dscGlobal.kMsg += btnStr + "4";
        else if (kByte2 == five)
          dscGlobal.kMsg += btnStr + "5";
        else if (kByte2 == six)
          dscGlobal.kMsg += btnStr + "6";
        else if (kByte2 == seven)
          dscGlobal.kMsg += btnStr + "7";
        else if (kByte2 == eight)
          dscGlobal.kMsg += btnStr + "8";
        else if (kByte2 == nine)
          dscGlobal.kMsg += btnStr + "9";
        else if (kByte2 == aster)
          dscGlobal.kMsg += btnStr + "*";
        else if (kByte2 == zero)
          dscGlobal.kMsg += btnStr + "0";
        else if (kByte2 == pound)
          dscGlobal.kMsg += btnStr + "#";
        else if (kByte2 == stay)
          dscGlobal.kMsg += btnStr + F("Stay");
        else if (kByte2 == away)
          dscGlobal.kMsg += btnStr + F("Away");
        else if (kByte2 == chime)
          dscGlobal.kMsg += btnStr + F("Chime");
        else if (kByte2 == reset)
          dscGlobal.kMsg += btnStr + F("Reset");
        else if (kByte2 == kExit)
          dscGlobal.kMsg += btnStr + F("Exit");
        else if (kByte2 == lArrow)  // These arrow commands don't work every time
          dscGlobal.kMsg += btnStr + F("<");
        else if (kByte2 == rArrow)  // They are often reverse for unknown reasons
          dscGlobal.kMsg += btnStr + F(">");
        else if (kByte2 == kOut)
          dscGlobal.kMsg += F("[Keypad Response]");
        else {
          dscGlobal.kMsg += "[Keypad] 0x" + String(kByte2, HEX) + " (Unknown)";
        }
      }

      if (cmd == fire)
        dscGlobal.kMsg += btnStr + F("Fire");
      if (cmd == aux)
        dscGlobal.kMsg += btnStr + F("Auxillary");
      if (cmd == panic)
        dscGlobal.kMsg += btnStr + F("Panic");
      
      return cmd;     // Return success
    }
  }

/* This is the interrupt handler used by this class. It is called every time the input
 * pin changes from high to low or from low to high.
 *
 * The function is not a member of the DSC class, it must be in the global scope in order 
 * to be called by attachInterrupt() from within the DSC class.
 */
void clkCalled_Handler()
  {
    dscGlobal.clockChange = micros();                   // Save the current clock change time
    dscGlobal.intervalTimer = 
        (dscGlobal.clockChange - dscGlobal.lastChange); // Determine interval since last clock change
    //if (intervalTimer > NEW_WORD_INTV) newWord = true;
    //else newWord = false;
    if (dscGlobal.intervalTimer > (NEW_WORD_INTV - 200)) {
      dscGlobal.kWord = dscGlobal.kBuild;               // Save the complete keypad raw data bytes sentence
      dscGlobal.kBuild = "";                            // Reset the raw data bytes keypad word being built
    }
    dscGlobal.lastChange = dscGlobal.clockChange;       // Re-save the current change time as last change time

    if (digitalRead(CLK)) {                 // If clock line is going HIGH, this is PANEL data
      dscGlobal.lastRise = dscGlobal.lastChange;        // Set the lastRise time
      if (dscGlobal.pBuild.length() <= MAX_BITS) {      // Limit the string size to something manageable
        //delayMicroseconds(120);           // Delay for 120 us to get a valid data line read
        if (digitalRead(DTA_IN)) dscGlobal.pBuild += "1"; 
        else dscGlobal.pBuild += "0";
      }
    }
    else {                                  // Otherwise, it's going LOW, this is KEYPAD data
      dscGlobal.lastFall = dscGlobal.lastChange;          // Set the lastFall time
      if (dscGlobal.kBuild.length() <= MAX_BITS) {        // Limit the string size to something manageable 
        //delayMicroseconds(200);           // Delay for 300 us to get a valid data line read
        if (digitalRead(DTA_IN)) dscGlobal.kBuild += "1"; 
        else dscGlobal.kBuild += "0";
      }
    }
  }

// int pnlBinary

// int kpdBinary

int DSC::pnlChkSum(String &dataStr)
  {
    // Sums all but the last full byte (minus padding) and compares to last byte
    // returns 0 if not valid, and 1 if checksum valid
    int cSum = 0;
    if (dataStr.length() > 8) {
      cSum += binToInt(dataStr,0,8);
      int grps = (dataStr.length() - 9) / 8;
      for(int i=0;i<grps;i++) {
        if (i<(grps-1)) 
          cSum += binToInt(dataStr,9+(i*8),8);
        else {
          byte cSumMod = cSum / 256;
          //String cSumStr = String(chkSum, HEX);
          //int cSumLen = cSumStr.length();
          byte lastByte = binToInt(dataStr,9+(i*8),8);
          //byte cSumByte = binToInt(cSumStr,(cSumLen-2),2);
          //if (cSumSub == lastByte) return true;
          //Serial.println(cSum);
          //Serial.println(cSumMod);
          //Serial.println(lastByte);
          if (cSumMod == lastByte) return 1;
        }
      }
    }
    return 0;
  }

unsigned int DSC::binToInt(String &dataStr, int offset, int dataLen)
  {
    // Returns the value of the binary data in the String from "offset" to "dataLen" as an int
    int iBuf = 0;
    for(int j=0;j<dataLen;j++) {
      iBuf <<= 1;
      if (dataStr[offset+j] == '1') iBuf |= 1;
    }
    return iBuf;
  }

String DSC::byteToBin(byte b)
  {
    // Returns the 8 bit binary representation of byte "b" with leading zeros as a String
    int zeros = 8-String(b, BIN).length();
    String zStr = "";
    for (int i=0;i<zeros;i++) zStr += "0";
    return zStr + String(b, BIN);
  }

void DSC::zeroArr(byte byteArr[])
  {
    // Zeros an array of ARR_SIZE  bytes
    for (int i=0;i<ARR_SIZE;i++) byteArr[i] = 0;
  }

void DSC::setCLK(int p)
  {
    // Sets the clock pin, must be called prior to begin()
    CLK = p;
  }

void DSC::setDTA_IN(int p)
  {
    // Sets the data in pin, must be called prior to begin()
    DTA_IN = p;
  }
  
void DSC::setDTA_OUT(int p)
  {
    // Sets the data out pin, must be called prior to begin()
    DTA_OUT = p;
  }
void DSC::setLED(int p)
  {
    // Sets the LED pin, must be called prior to begin()
    LED = p;
  }

size_t DSC::write(uint8_t character) 
  { 
    // Code to display letter when given the ASCII code for it
  }

size_t DSC::write(const char *str) 
  { 
    // Code to display string when given a pointer to the beginning -- 
    // remember, the last character will be null, so you can use a while(*str). 
    // You can increment str (str++) to get the next letter
  }
  
size_t DSC::write(const uint8_t *buffer, size_t size) 
  { 
    // Code to display array of chars when given a pointer to the beginning 
    // of the array and a size -- this will not end with the null character
  }

///////// OLD //////////
/*
int TextBuffer::clear() 
  {
    if (!buffer) return 0;        // return failure
    buffer[0] = (char)0;
    position = 0;
    length = 0;
    return 1;                     // return success
  } 

int TextBuffer::end()
  {
    if (!buffer) return 0;        // return failure
    free(buffer);
    return 1;                     // return success
  }

const char* TextBuffer::getBuffer() 
  {
    if (!buffer) return (char)0;  // return null terminator
    return (const char*)buffer;   // return const char array buffer pointer
  }
  
char* TextBuffer::getBufPointer() 
  {
    if (!buffer) return 0;        // return failure
    clear();                      // clear the buffer
    return (char*)buffer;         // return char array buffer pointer
  }
  
int TextBuffer::getSize()
  {
    if (!buffer) return 0;        // return failure
      if (strlen((const char*)buffer) != length)
        // if the length is not correct, probably due to an external
        // write to the buffer, reset it to the correct length
        {
          length = strlen((const char*)buffer);
        }
    return length;
  }

int TextBuffer::getCapacity()
  {
    if (!buffer) return 0;        // return failure
    return capacity;
  }
  
String TextBuffer::getCheckSum()
  {
    // Create Checksum
    char checkSum = 0;
    int csCount = 1;
    while (buffer[csCount + 1] != 0)
    {
      checkSum ^= buffer[csCount];
      csCount++;
    }
    // Change the checksum to a string, in HEX form, convert to upper case, and print
    String checkSumStr = String(checkSum, HEX);
    checkSumStr.toUpperCase();
    
    return checkSumStr;
  }
  
*/