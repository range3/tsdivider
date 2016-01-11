#ifndef _TSSP_TOT_HPP_
#define _TSSP_TOT_HPP_

#include <chrono>
#include "aribtime.hpp"
#include "util.hpp"

namespace tssp
{
using std::chrono::system_clock;

struct time_offset_table
{
  uint8_t table_id; // TDT: 0x70, TOT: 0x73
  aribtime time;

public:
  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    if(size < 8)
      std::runtime_error("");

    table_id = get8(p);
    p += 3;
    time.unpack(&p, pend);
  }
};

}


#endif
