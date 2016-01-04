#ifndef _TSSP_UTIL_HPP_
#define _TSSP_UTIL_HPP_

#include <cstdint>
#include <ostream>
#include <iomanip>

namespace tssp
{

inline
uint8_t get8(const uint8_t* p) {
  return *p;
}

inline
uint16_t get16(const uint8_t* p) {
  return (get8(p) << 8) | get8(p+1);
}

inline
uint32_t get32(const uint8_t* p) {
  return (get16(p) << 16) | get16(p+2);
}

inline
uint8_t get8(const char* p) {
  return *reinterpret_cast<const uint8_t*>(p);
}

inline
uint16_t get16(const char* p) {
  return get16(reinterpret_cast<const uint8_t*>(p));
}

inline
uint32_t get32(const char* p) {
  return get32(reinterpret_cast<const uint8_t*>(p));
}

inline
void hexdump(const char* data, size_t size, ostream& ost) {
  size_t i;
  for(i = 0; i < size; ++i){
    ost 
      << std::setw(2)
      << std::setfill('0')
      << std::hex
      << std::uppercase
      << static_cast<int>(get8(data+i))
      << ' ';
    if(i % 16 == 15)
      ost << endl;
  }
  if(i % 16 != 0)
    ost << endl;
}

}

#endif
