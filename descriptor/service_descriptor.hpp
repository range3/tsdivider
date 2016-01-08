#ifndef _TSSP_SERVICE_DESCRIPTOR_
#define _TSSP_SERVICE_DESCRIPTOR_

#include <string>
#include "util.hpp"

namespace tssp
{

class service_descriptor
{
public:
  static const uint8_t TAG = 0x48;

public:
  uint8_t service_type;
  std::string service_provider_name;
  std::string service_name;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    if(pend - p < 2)
      std::runtime_error("");

    service_type = get8(p);
    p += 1;
    size_t length = get8(p);
    p += 1;

    if(pend - p < length)
      std::runtime_error("");
    service_provider_name.assign(p, p+length);
    p += length;

    if(pend - p < 1)
      std::runtime_error("");
    length = get8(p);
    p += 1;

    if(pend - p < length)
      std::runtime_error("");
    service_name.assign(p, p+length);
    p += length;

    *pp = p;
  }
};



}


#endif
