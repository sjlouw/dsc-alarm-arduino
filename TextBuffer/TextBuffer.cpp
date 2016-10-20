#include "Arduino.h"
#include "TextBuffer.h"

TextBuffer::TextBuffer(unsigned int bufSize)
  {
    _bufSize = (bufSize + 3) & (~3);  // Makes sure size is a multiple of 4
                                      //   - required for the ESP8266 boards
  }

int TextBuffer::begin()
  {
    buffer = (byte*)malloc(sizeof(byte)*_bufSize);
    
    if (!buffer) return 0;        // return failure if malloc fails
    capacity = _bufSize;
    memset(buffer, 0, capacity);  // Initialize by zeroing the entire array
    position = 0;                 // -- Not currently used --
    return capacity;              // return buffer capacity if successful
  }

size_t TextBuffer::write(uint8_t character) 
  {
    if (!buffer) return 0;        // return failure
    if((getSize() + 1) < capacity) {
      // Save the character to the end of the buffer, if there is room
      buffer[getSize()] = character;
      return 1;                   // return success (1 byte)
    }
    return 0;                     // return failure (buffer full)
  }

size_t TextBuffer::write(const char *str) 
  { 
    // Code to display/add string when given a pointer to the beginning
    // The last character will be null, so a while(*str) is used
    // Increment str (str++) to get the next letter
    if (!buffer) return 0;        // return failure
    if (str == NULL) return 0;      
    return write((const uint8_t *)str, strlen(str));
  }
    
size_t TextBuffer::write(const uint8_t *wBuffer, size_t size) 
  { 
    // Code to display/add array of chars when given a pointer to the 
    // beginning of the array and a size. This will not end with the null character
    if (!buffer) return 0;        // return failure
    size_t n = 0;  
    while (size--) {    
      if (write(*wBuffer++)) n++;    
      else break;  
    }  
    return n;                     // Return the number of bytes written
  }

int TextBuffer::clear() 
  {
    if (!buffer) return 0;        // return failure
    memset(buffer, 0, capacity);  // Zero the entire array
    position = 0;
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
    if (!buffer) return (char)0;  // return 0 byte (null terminator)
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
    return strlen((const char*)buffer);
  }

int TextBuffer::getCapacity()
  {
    if (!buffer) return 0;        // return failure
    return capacity;
  }
 
int TextBuffer::getCheckSum()
  {
    if (!buffer) return 0;        // return failure
    // Create Checksum
    int checkSum = 0;
    int csCount = 1;
    while (buffer[csCount + 1] != 0)
    {
      checkSum ^= buffer[csCount];
      csCount++;
    }
    /*
    // Change the checksum to a string, in HEX form, convert to upper case, and return
    String checkSumStr = String(checkSum, HEX);
    checkSumStr.toUpperCase();
    return checkSumStr; 
    */
    return checkSum;        // Return the checksum as type int
}
