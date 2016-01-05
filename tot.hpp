#ifndef _TSSP_TOT_HPP_
#define _TSSP_TOT_HPP_

#include <chrono>
#include "util.hpp"

namespace tssp
{
using std::chrono::system_clock;

struct time_offset_table
{
  uint8_t table_id; // TDT: 0x70, TOT: 0x73
  uint16_t mjd;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;

public:
  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    if(size < 8)
      std::runtime_error("");

    table_id = get8(p);
    p += 3;
    mjd = get16(p);
    p += 2;
    hour = get8(p);
    p += 1;
    min = get8(p);
    p += 1;
    sec = get8(p);
  }

  system_clock::time_point time() const {
    return system_clock::from_time_t(
        mjd_to_time_t(mjd) + hour*60*60 + min*60 + sec - 9*60*60);
  }

private:
  time_t mjd_to_time_t(uint16_t mjd) const {
    return (mjd - 40587)*86400;
  }
  
};

}


#endif
