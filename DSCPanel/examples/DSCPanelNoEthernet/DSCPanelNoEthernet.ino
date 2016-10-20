// DSC_18XX Arduino Interface - Example with no Ethernet functionality
//
// Sketch to decode the keybus protocol on DSC PowerSeries 1816, 1832 and 1864 panels
//   -- Use the schematic at https://github.com/emcniece/Arduino-Keybus to connect the
//      keybus lines to the arduino via voltage divider circuits.  Don't forget to 
//      connect the Keybus Ground to Arduino Ground (not depicted on the circuit)! You
//      can also power your arduino from the keybus (+12 VDC, positive), depending on the 
//      the type arduino board you have.
//
//

#include <TextBuffer.h>
#include <TimeLib.h>
#include <DSC.h>

DSC dsc;                              // Initialize DSC.h library as "dsc"
TextBuffer message(128);              // Initialize TextBuffer.h for print/client message

////////// Functions that need declarations prior to Main ///////////

int pnlBinary(String &dataStr)
{
  // Formats the referenced string into bytes of binary data in the form:
  //  8 1 8 8 8 8 8 etc, and then prints each segment 
  int cSumOk = 0; 
  if (dataStr.length() > 8) {
    message.print(dataStr.substring(0,8));
    message.print(" ");
    message.print(dataStr.substring(8,9));
    message.print(" ");
    int grps = (dataStr.length() - 9) / 8;
    for(int i=0;i<grps;i++) {
      message.print(dataStr.substring(9+(i*8),9+(i+1)*8));
      message.print(" ");
    }
    if (dataStr.length() > ((grps*8)+9))
      message.print(dataStr.substring((grps*8)+9,dataStr.length()));
  }
  else {
    message.print(dataStr);
  }
  if (dsc.pnlChkSum(dataStr)) {
    message.print(" (OK)");
    cSumOk = 1;
  }
  message.println();
  return cSumOk;
}

int kpdBinary(String &dataStr)
{
  // Formats the referenced string into bytes of binary data in the form:
  //  8 8 8 8 8 8 etc, and then prints each segment
  int cSumOk = 0; 
  if (dataStr.length() > 8) {
    int grps = dataStr.length() / 8;
    for(int i=0;i<grps;i++) {
      message.print(dataStr.substring(i*8,(i+1)*8));
      message.print(" ");
    }
    if (dataStr.length() > (grps*8))
      message.print(dataStr.substring((grps*8),dataStr.length()));
  }
  else {
    message.print(dataStr);
  }
  message.println();
  return cSumOk;
}

/////////// ------------------------------------------- /////////////

void setup()
{ 
  Serial.begin(115200);
  Serial.flush();
  Serial.println(F("DSC Powerseries 18XX"));
  Serial.println(F("Key Bus Interface"));
  Serial.println(F("Initializing"));

  message.begin();              // Begin the message buffer, allocate memory
 
  dsc.setCLK(3);    // Sets the clock pin to 3 (example, this is also the default)
                    // setDTA_IN( ), setDTA_OUT( ) and setLED( ) can also be called
  dsc.begin();      // Start the dsc library (Sets the pin modes)
}

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------  MAIN LOOP  ---------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

void loop()
{  
 
  // --------------- Print No Data Message -------------- (FOR DEBUG PURPOSES)
  if ((millis() - dscGlobal.lastData) > 20000) {
    // Print no data message if there is no new data in XX time (ms)
    Serial.println(F("--- No data for 20 seconds ---"));  
    dscGlobal.lastData = millis();          // Reset the timer
  }

  // ---------------- Get/process incoming data ----------------
  if (!dsc.process()) return;

  if (dscGlobal.pCmd) {
    // ------------ Write the panel message to buffer ------------
    message.clear();                      // Clear the message Buffer (this sets first byte to 0)
    message.print(F("[Panel]  "));
    pnlBinary(dscGlobal.pWord);           // Writes formatted raw data to the message buffer

    // ------------ Print the formatted raw data ------------
    Serial.print(message.getBuffer());

    message.clear();                      // Clear the message Buffer (this sets first byte to 0)
    message.print(formatTime(now()));     // Add the time stamp
    message.print(" ");
    if (String(dscGlobal.pCmd,HEX).length() == 1)
      message.print("0");                 // Write a leading zero to a single digit HEX
    message.print(String(dscGlobal.pCmd,HEX));
    message.print("(");
    message.print(dscGlobal.pCmd);
    message.print("): ");
    message.println(dscGlobal.pMsg);
  
    // ------------ Print the message ------------
    Serial.print(message.getBuffer());
  }

  if (dscGlobal.kCmd) {
    // ------------ Write the keypad message to buffer ------------
    message.clear();                      // Clear the message Buffer (this sets first byte to 0)
    message.print(F("[Keypad] "));
    kpdBinary(dscGlobal.kWord);           // Writes formatted raw data to the message buffer

    // ------------ Print the formatted raw data ------------
    Serial.print(message.getBuffer());
  
    message.clear();                      // Clear the message Buffer (this sets first byte to 0)
    message.print(formatTime(now()));     // Add the time stamp
    message.print(" ");
    if (String(dscGlobal.kCmd,HEX).length() == 1)
      message.print("0");                 // Write a leading zero to a single digit HEX
    message.print(String(dscGlobal.kCmd,HEX));
    message.print("(");
    message.print(dscGlobal.kCmd);
    message.print("): ");
    message.println(dscGlobal.kMsg);

    // ------------ Print the message ------------
    Serial.print(message.getBuffer());
  }
}

// -----------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------  FUNCTIONS  ---------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------

String formatTime(time_t cTime)
{
  return digits(hour(cTime)) + ":" + digits(minute(cTime)) + ":" + digits(second(cTime)) + ", " +
         String(month(cTime)) + "/" + String(day(cTime)) + "/" + String(year(cTime));  //+ "/20" 
}

String digits(unsigned int val)
{
  // Take an unsigned int and convert to a string with appropriate leading zeros
  if (val < 10) return "0" + String(val);
  else return String(val);
}

// -----------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------  END  ------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------
