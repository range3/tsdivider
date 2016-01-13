#ifndef _TSSP_SHORT_EVENT_DESCRIPTOR_
#define _TSSP_SHORT_EVENT_DESCRIPTOR_

#include <string>
#include "util.hpp"

namespace tssp
{

class short_event_descriptor
{
public:
  static const uint8_t TAG = 0x4D;

public:
  char iso_639_language_code[4];
  std::string event_name;
  std::string text;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;

    if(pend - p < 4)
      std::runtime_error("");

    iso_639_language_code[0] = get8(p);
    p += 1;
    iso_639_language_code[1] = get8(p);
    p += 1;
    iso_639_language_code[2] = get8(p);
    p += 1;
    iso_639_language_code[3] = '\0';

    size_t length = get8(p);
    p += 1;

    if(pend - p < length)
      std::runtime_error("");
    event_name.assign(p, p+length);
    p += length;

    if(pend - p < 1)
      std::runtime_error("");
    length = get8(p);
    p += 1;
    if(pend - p < length)
      std::runtime_error("");
    text.assign(p, p+length);
    p += length;

    *pp = p;
  }
};



}


#endif
