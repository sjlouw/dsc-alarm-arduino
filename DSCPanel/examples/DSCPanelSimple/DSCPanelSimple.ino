// DSC_18XX Arduino Interface - Simple Example
//
// - Demonstrates the use of the DSC library, prints the raw binary words and 
//   minimally formats and then prints the decoded panel message
//
// Sketch to decode the keybus protocol on DSC PowerSeries 1816, 1832 and 1864 panels
//   -- Use the schematic at https://github.com/emcniece/Arduino-Keybus to connect the
//      keybus lines to the arduino via voltage divider circuits.  Don't forget to 
//      connect the Keybus Ground to Arduino Ground (not depicted on the circuit)! You
//      can also power your arduino from the keybus (+12 VDC, positive), depending on the 
//      the type arduino board you have.
//
//

#include <DSC.h>

DSC dsc;            // Initialize DSC.h library as "dsc"

// --------------------------------------------------------------------------------------------------------
// -----------------------------------------------  SETUP  ------------------------------------------------
// --------------------------------------------------------------------------------------------------------

void setup()
{ 
  Serial.begin(115200);
  Serial.flush();
  Serial.println(F("DSC Powerseries 18XX"));
  Serial.println(F("Key Bus Interface"));
  Serial.println(F("Initializing"));
 
  dsc.setCLK(3);    // Sets the clock pin to 3 (example, this is also the default)
                    // setDTA_IN( ), setDTA_OUT( ) and setLED( ) can also be called
  dsc.begin();      // Start the dsc library (Sets the pin modes)
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------  MAIN LOOP  ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

void loop()
{  
  // ---------------- Get/process incoming data ----------------
  if (!dsc.process()) return;

  if (dscGlobal.pCmd) {
    // ------------ Print the Binary Panel Word ------------
    //Serial.print(F("[Panel]  "));
    Serial.println(dsc.pnlRaw());

    // ------------ Print the Formatted Panel Word ------------
    Serial.println(dsc.pnlFormat());

    // ------------ Print the decoded Panel Message ------------
    Serial.print("---> ");
    if (String(dscGlobal.pCmd,HEX).length() == 1)
      Serial.print("0");                  // Write a leading zero to a single digit HEX
    Serial.print(String(dscGlobal.pCmd,HEX));
    Serial.print("(");
    Serial.print(dscGlobal.pCmd);
    Serial.print("): ");
    Serial.println(dscGlobal.pMsg);
  }

  if (dscGlobal.kCmd) {
    // ------------ Print the Binary Keypad Word ------------
    //Serial.print(F("[Keypad] "));
    Serial.println(dsc.kpdRaw());    

    // ------------ Print the Formatted Keypad Word ------------
    Serial.println(dsc.kpdFormat());

    // ------------ Print the decoded Keypad Message ------------
    Serial.print("---> ");
    if (String(dscGlobal.kCmd,HEX).length() == 1)
      Serial.print("0");                  // Write a leading zero to a single digit HEX
    Serial.print(String(dscGlobal.kCmd,HEX));
    Serial.print("(");
    Serial.print(dscGlobal.kCmd);
    Serial.print("): ");
    Serial.println(dscGlobal.kMsg);
  }
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------  FUNCTIONS  ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

// None

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------------  END  -------------------------------------------------
// --------------------------------------------------------------------------------------------------------
