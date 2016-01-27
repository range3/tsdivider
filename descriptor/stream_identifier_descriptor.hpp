#ifndef _TSD_STREAM_IDENTIFIER_DESCRIPTOR_HPP_
#define _TSD_STREAM_IDENTIFIER_DESCRIPTOR_HPP_

#include "util.hpp"

namespace tsd
{

class stream_identifier_descriptor
{
public:
  static const uint8_t TAG = 0x52;

public:
  uint8_t component_tag;

public:
  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    if(pend - p < 1)
      std::runtime_error("");

    component_tag = get8(p);
    p += 1;

    *pp = p;
  }
};

}

#endif
