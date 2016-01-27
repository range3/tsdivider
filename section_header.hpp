#ifndef _TSD_SECTION_HEADER_HPP_
#define _TSD_SECTION_HEADER_HPP_

#include <cstdint>
#include "util.hpp"

namespace tsd
{

class section_header
{
public:
  uint8_t table_id;
  uint8_t section_syntax_indicator;
  uint16_t section_length;
  uint16_t table_id_extension;
  uint8_t version;
  uint8_t current_next_indicator;
  uint8_t section_number;
  uint8_t last_section_number;

public:
  void unpack(
      const uint8_t** pp,
      const uint8_t* pend) {
    const uint8_t* p = *pp;

    if(pend - p < 3)
      throw std::runtime_error("");

    table_id = get8(p);
    p += 1;
    section_syntax_indicator = get8(p) >> 7;
    section_length = get16(p) & 0x0FFF;
    p += 2;
    if(section_syntax_indicator == 0) {
      *pp = p;
      return;
    }

    if(pend - p < 5)
      throw std::runtime_error("");

    table_id_extension = get16(p);
    p += 2;
    version = (get8(p) >> 1) & 0x1F;
    current_next_indicator = get8(p) & 0x01;
    p += 1;
    section_number = get8(p);
    p += 1;
    last_section_number = get8(p);
    p += 1;

    *pp = p;
  }

  size_t get_payload_length() const {
    if(section_syntax_indicator)
      return section_length-5;
    else
      return section_length;
  }
};

}


#endif
