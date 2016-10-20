/*
  TextBuffer.h - Library for writing text/characters to a buffer
  that will store them in a char array of predetermined length
  
  Inherits from the Print.h class and uses the predefined print
  methods to feed this class
  
  Created by stagf15, 11 Oct 16.
  Released into the public domain.
  
*/

#ifndef TextBuffer_h
#define TextBuffer_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


class TextBuffer : public Print
{
  public:
    
    // In general, functions will return the following:
    //    If failure, return int 0, or (char)0
    //    If success, return int 1, or the byte size of 
    //      what was added/changed/created
    
    // If malloc fails in begin() then it will return 0
    //   - all other functions will then return failure
    //   - if this would cause failure in the sketch, recommend
    //       checking begin() for failure and handling there
    
    // Class to call to initialize the buffer (before Setup). Requires the 
    // desired size of the buffer in bytes. The size is then checked and 
    // converted, if required, to a multiple of 4. (for ESP8266 boards)
    TextBuffer(unsigned int bufSize);
    
    // Begins the buffer; allocates the memory (in Setup)
    int begin();
    
    // Functions used by the extension of print class
    virtual size_t write(uint8_t);
    virtual size_t write(const char *str);
    virtual size_t write(const uint8_t *wBuffer, size_t size);

    // Clears the buffer, writes 0 bytes to all elements (memset) 
    // and resets position to 0
    int clear();
    
    // Ends, or de-allocates the buffer and frees the memory   
    // Requires a begin() to create a new buffer if needed
    int end();
    
    // Returns a pointer to the buffer as a const char array
    //   Used to return the buffer itself
    const char* getBuffer();
    
    // Returns a pointer to the buffer as a char array
    //   Used to modify the buffer directly from the calling sketch
    //   - this could be dangerous, if the capacity is not checked prior!
    //   - calling this will overwrite anything in the buffer
    char* getBufPointer();
    
    // Returns the size (length) of the buffer, not including null terminator
    int getSize();
    
    // Returns the capacity of the buffer, equal to the bufsize passed initially
    int getCapacity();
    
    // Creates a checksum from the sequential XOR of the buffer characters
    // - This is the NMEA0183 standard checksum
    int getCheckSum();
    
  private:  
  
    // Pointer to the buffer char array
    byte* buffer;
    
    // Variable to hold the aligned buffer max capacity
    // - Aligned to multiples of 4 bytes for the ESP8266 boards
    unsigned int _bufSize;
    
    // Same as _bufSize, the max capacity, including null terminator, of the buffer
    unsigned int capacity;
    
    // The position of the "cursor", or reference point in the buffer (usually 0)
    // - Not currently used
    unsigned int position;
};

#endif
