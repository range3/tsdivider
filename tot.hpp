#ifndef _TSSP_TOT_HPP_
#define _TSSP_TOT_HPP_

#include <chrono>
#include "aribtime.hpp"
#include "util.hpp"

namespace tssp
{
using std::chrono::system_clock;

// TDT: 0x70, TOT: 0x73
struct time_offset_table
{
  aribtime time;

public:
  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    if(size < 5)
      std::runtime_error("");

    time.unpack(&p, pend);
  }
};

}


#endif
