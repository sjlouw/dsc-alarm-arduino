/* DSC_Constants.h
 * Part of DSC Library 
 * See COPYRIGHT.txt and LICENSE.txt for more information.
 *
 * It contains all of the constants used by the DSC class.
 * They have to be declared global in scope because they may be accessed by
 * the ISR and you cannot pass parameters nor objects to an ISR routine.
 * 
 * In general, applications would not include this file. 
 */
 
#ifndef DSC_Constants_h
#define DSC_Constants_h

// ----- Word/Timing Constants -----
const byte MAX_BITS = 200;        // Max of 254
const int NEW_WORD_INTV = 5200;   // New word indicator interval in us (Microseconds)
const byte ARR_SIZE = 14;         // Max of 254   // NOT USED

// ------ HEX LOOK-UP ARRAY ------
const char hex[] = "0123456789abcdef";  // HEX alphanumerics look-up array

// ----- KEYPAD BUTTON VALUES -----
const byte kOut   = 0xff;   // 11111111 Usual 1st byte from keypad
const byte k_ff   = 0xff;   // 11111111 Keypad CRC checksum 1?
const byte k_7f   = 0x7f;   // 01111111 Keypad CRC checksum 2?
// The following buttons data are in the 2nd byte:
const byte one    = 0x82;   // 10000010
const byte two    = 0x85;   // 10000101
const byte three  = 0x87;   // 10000111
const byte four   = 0x88;   // 10001000
const byte five   = 0x8b;   // 10001011
const byte six    = 0x8d;   // 10001101
const byte seven  = 0x8e;   // 10001110
const byte eight  = 0x91;   // 10010001
const byte nine   = 0x93;   // 10010011
const byte aster  = 0x94;   // 10010100
const byte zero   = 0x80;   // 10000000
const byte pound  = 0x96;   // 10010110
const byte stay   = 0xd7;   // 11010111
const byte away   = 0xd8;   // 11011000
const byte chime  = 0xdd;   // 11011101
const byte reset  = 0xed;   // 11101101
const byte kExit  = 0xf0;   // 11110000
const byte lArrow = 0xfb;   // 11111011
const byte rArrow = 0xf7;   // 11110111
// The following button's data are in the 1st byte, and these 
//   seem to be sent twice, with a panel response in between:
const byte fire   = 0xbb;   // 10111011 
const byte aux    = 0xdd;   // 11011101 
const byte panic  = 0xee;   // 11101110

#endif