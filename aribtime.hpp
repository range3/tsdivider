#ifndef _TSSP_ARIBTIME_HPP_
#define _TSSP_ARIBTIME_HPP_

#include <chrono>
#include "util.hpp"

namespace tssp
{
using std::chrono::system_clock;

struct aribtime
{
  uint16_t mjd;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
public:
  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    unpack(&p, p+size);
  }

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    if(pend - p < 5)
      std::runtime_error("");

    mjd = get16(p);
    p += 2;
    hour = get8(p);
    p += 1;
    min = get8(p);
    p += 1;
    sec = get8(p);
    p += 1;
    *pp = p;
  }

  time_t to_time_t() const {
    return mjd_to_time_t(mjd) + hour*60*60 + min*60 + sec - 9*60*60;
  }

  system_clock::time_point time() const {
    return system_clock::from_time_t(to_time_t());
  }

private:
  time_t mjd_to_time_t(uint16_t mjd) const {
    return (mjd - 40587)*86400;
  }

};

}

#endif
