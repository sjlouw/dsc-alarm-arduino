// DSC_18XX Arduino Interface - Example with Ethernet Functionality
//
// Sketch to decode the keybus protocol on DSC PowerSeries 1816, 1832 and 1864 panels
//   -- Use the schematic at https://github.com/emcniece/Arduino-Keybus to connect the
//      keybus lines to the arduino via voltage divider circuits.  Don't forget to 
//      connect the Keybus Ground to Arduino Ground (not depicted on the circuit)! You
//      can also power your arduino from the keybus (+12 VDC, positive), depending on the 
//      the type arduino board you have.
//
//

#include <SPI.h>
#include <Ethernet.h>
#include <TextBuffer.h>
#include <TimeLib.h>
#include <DSC.h>

// ----- Ethernet/WiFi Variables -----
bool newClient = false;               // Whether the client is new or not
bool streamData = false;              // Was the request to stream data? (/STREAM)
// Enter a MAC address and IP address for the controller:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set a manual IP address in case DHCP Fails:
IPAddress ip(192, 168, 1, 169);

DSC dsc;                              // Initialize DSC.h library as "dsc"
TextBuffer message(128);              // Initialize TextBuffer.h for print/client message
TextBuffer timeBuf(24);               // Initialize TextBuffer.h for formatted time message

EthernetServer server(80);            // Start Ethernet Server on Port 80
EthernetClient client;                // Start Ethernet Client

// --------------------------------------------------------------------------------------------------------
// -----------------------------------------------  SETUP  ------------------------------------------------
// --------------------------------------------------------------------------------------------------------

void setup()
{ 
  Serial.begin(115200);
  Serial.flush();
  Serial.println(F("DSC Powerseries 18XX"));
  Serial.println(F("Key Bus Monitor"));
  Serial.println(F("Initializing"));

  message.begin();              // Begin the message buffer, allocate memory
  timeBuf.begin();              // Begin the formatted time buffer, allocate memory
 
  // Start the Ethernet connection and the server (Try to use DHCP):
  Serial.println(F("Trying to get an IP address using DHCP..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Trying to manually set IP address..."));
    // Initialize the ethernet device using manual IP address:
    if (1 == 0) {               // This logic is "shut off", doesn't work with Ethernet client
      Serial.println(F("No ethernet connection"));
    }
    else {
      Ethernet.begin(mac, ip);
      server.begin();
    }
  }
  Serial.print(F("Server is at: "));
  Serial.println(Ethernet.localIP());
  Serial.println();

  dsc.setCLK(3);    // Sets the clock pin to 3 (example, this is also the default)
                    // setDTA_IN( ), setDTA_OUT( ) and setLED( ) can also be called
  dsc.begin();      // Start the dsc library (Sets the pin modes)
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------  MAIN LOOP  ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

void loop()
{  
  // -------------- Check for a new connection --------------
  if (!client.connected()) {
    client = server.available();
    newClient = true;
    streamData = false;
  }
  if (client and newClient) {
    float connTimeout = millis() + 500;    // Set the timeout 500 ms from now
    while(true) {
      // If there is available data
      if (client.available()) {
        // Read the first two words of the request
        String request_type = client.readStringUntil(' ');  // Read till the first space
        String request = client.readStringUntil(' ');       // Read till the next space
        Serial.print(F("Request: "));
        Serial.print(request_type);
        Serial.print(" ");
        Serial.println(request);
        //client.flush();
        if (request == "/STREAM") {
          streamData = true;
          newClient = false;
          for (int i=0; i <= 30; i++){
            client.println(F("Empty --------------------------------------------------"));
          } 
        }
        else {
          streamData = false;
          client.stop();
          newClient = true;
        }
        break;
      }
      // If not, wait up to [connTimeout] ms for data
      if (millis() >= connTimeout) {
        Serial.println(F("Connection Timeout"));
        streamData = false;
        client.stop();
        newClient = true;
      }
      delay(5);
    }
  }
 
  // --------------- Print No Data Message -------------- (FOR DEBUG PURPOSES)
  if ((millis() - dscGlobal.lastData) > 20000) {
    // Print no data message if there is no new data in XX time (ms)
    Serial.println(F("--- No data for 20 seconds ---"));  
    if (client.connected() and streamData) {
      client.println(F("--- No data for 20 seconds ---")); 
    }
    dscGlobal.lastData = millis();          // Reset the timer
  }

  // ---------------- Get/process incoming data ----------------
  if (!dsc.process()) return;

  if (dsc.timeAvailable) setDscTime();    // Attempt to update the system time

  if (dscGlobal.pCmd) {
    // ------------ Print the formatted raw data ------------
    //Serial.print(message.getBuffer());  // Prints unformatted word to serial
    Serial.println(dsc.pnlFormat());

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
    if (client.connected() and streamData) {
      client.write(message.getBuffer(), message.getSize());
    }
  }

  if (dscGlobal.kCmd) {
    // ------------ Print the formatted raw data ------------
    //Serial.print(message.getBuffer());  // Prints unformatted word to serial
    Serial.println(dsc.kpdFormat());
  
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
    if (client.connected() and streamData) {
      client.write(message.getBuffer(), message.getSize());
    }
  }
}

// --------------------------------------------------------------------------------------------------------
// ---------------------------------------------  FUNCTIONS  ----------------------------------------------
// --------------------------------------------------------------------------------------------------------

const char* formatTime(time_t cTime)
{
  timeBuf.clear();
  timeBuf.print(digits(hour(cTime)));     timeBuf.print(":");
  timeBuf.print(digits(minute(cTime)));   timeBuf.print(":");
  timeBuf.print(digits(second(cTime)));   timeBuf.print(", ");
  timeBuf.print(month(cTime));            timeBuf.print("/");
  timeBuf.print(day(cTime));              timeBuf.print("/");
  timeBuf.print(year(cTime)); 

  return timeBuf.getBuffer();
}

/* // OLD formatTime - String was unstable
String formatTime(time_t cTime)
{
  return digits(hour(cTime)) + ":" + digits(minute(cTime)) + ":" + digits(second(cTime)) + ", " +
         String(month(cTime)) + "/" + String(day(cTime)) + "/" + String(year(cTime));  //+ "/20" 
}
*/

String digits(unsigned int val)
{
  // Take an unsigned int and convert to a string with appropriate leading zeros
  if (val < 10) return "0" + String(val);
  else return String(val);
}

void setDscTime()
{
  setTime(dsc.HH,dsc.MM,dsc.SS,dsc.dd,dsc.mm,dsc.yy);
  if (timeStatus() == timeSet) {
    Serial.println(F("Time Synchronized"));
  }
  else {
    Serial.println(F("Time Sync Error"));
  } 
}

// --------------------------------------------------------------------------------------------------------
// ------------------------------------------------  END  -------------------------------------------------
// --------------------------------------------------------------------------------------------------------
