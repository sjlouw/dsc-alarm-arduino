/* DSC_Globals.h
 * Part of DSC Library 
 * See COPYRIGHT.txt and LICENSE.txt for more information.
 *
 * It contains definition of global items which are used by the DSC class.
 * They have to be declared global in scope because they are accessed by
 * the ISR and you cannot pass parameters nor objects to an ISR routine.
 * 
 * In general, applications would not include this file. 
 */
#ifndef DSC_Globals_h
#define DSC_Globals_h
#include <Arduino.h>

/* Timing data is stored in a buffer by the receiver object. It is an array of
 * uint16_t that should be at least 100 entries as defined by this default below.
 * However some IR sequences will require longer buffers especially those used for
 * air conditioner controls. In general we recommend you keep this value below 255
 * so that the index into the array can remain 8 bits. This library can handle larger 
 * arrays however it will make your code longer in addition to taking more RAM.

#define PNL_BUF_LENGTH 128
#define KPD_BUF_LENGTH 128
#if (PNL_BUF_LENGTH > 255)
	typedef uint16_t bufIndex_t;
#else
	typedef uint8_t bufIndex_t;
#endif
#if (KPD_BUF_LENGTH > 255)
	typedef uint16_t bufIndex_t;
#else
	typedef uint8_t bufIndex_t;
#endif
 */

// ----- Input/Output Pins -----
extern byte CLK;         // Keybus Yellow (Clock Line)
extern byte DTA_IN;      // Keybus Green (Data Line via V divider)
extern byte DTA_OUT;     // Keybus Green Output (Data Line through driver)
extern byte LED;         // LED pin on the arduino

/*
// Receiver states. This previously was enum but changed it to uint8_t
// to guarantee it was a single atomic 8-bit value.
#define  STATE_UNKNOWN 0
#define  STATE_NEW_WORD_MARK 1
#define  STATE_TIMING_MARK 2
#define  STATE_TIMING_SPACE 3
#define  STATE_FINISHED 4
#define  STATE_RUNNING 5
typedef uint8_t  currentState_t;
*/

/* The structure contains information used by the ISR routine. Because we cannot
 * pass parameters to an ISR, vars must be global. Values which can be changed by
 * the ISR but are accessed outside the ISR must be volatile (for the most part)
 */
 
typedef struct 
{  
  // ----- Keybus Word String Vars -----
  String pBuild, pWord, oldPWord, pMsg;
  String kBuild, kWord, oldKWord, kMsg;
  byte pCmd, kCmd;
  
  // ----- Time Variables -----
  unsigned long lastStatus;
  unsigned long lastData;
  // Volatile time variables, modified within ISR
  volatile unsigned long intervalTimer;   
  volatile unsigned long clockChange;
  volatile unsigned long lastChange;      
  volatile unsigned long lastRise;        // NOT USED
  volatile unsigned long lastFall;        // NOT USED
  
  volatile bool newWord;                  // NOT USED
  
  // ----- Byte Arrays -----
  byte pBytes[];  // NOT USED
  byte kBytes[];  // NOT USED
} 
dscGlobal_t;
extern  dscGlobal_t dscGlobal;  //declared in DSC.cpp

#endif
