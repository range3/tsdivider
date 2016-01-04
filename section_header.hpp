#ifndef _TSSP_SECTION_HEADER_HPP_
#define _TSSP_SECTION_HEADER_HPP_

#include <cstdint>
#include "util.hpp"

namespace tssp
{

class section_header
{
public:
  uint8_t table_id;
  uint16_t transport_stream_id;
  uint8_t version;
  uint8_t section_number;
  uint8_t last_section_number;

public:
  void unpack(
      const uint8_t** pp,
      const uint8_t* pend) {
    const uint8_t* p = *pp;

    if(pend - p < 8)
      throw std::runtime_error("");

    table_id = get8(p);
    p += 3;
    transport_stream_id = get16(p);
    p += 2;
    version = (get8(p) >> 1) & 0x1F;
    p += 1;
    section_number = get8(p);
    p += 1;
    last_section_number = get8(p);
    p += 1;

    *pp = p;
  }
};

}


#endif
