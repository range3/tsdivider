#ifndef _TSSP_DESCRIPTOR_BASE_HPP_
#define _TSSP_DESCRIPTOR_BASE_HPP_

namespace tssp
{
struct descriptor_header
{
  uint8_t tag;
  uint8_t length;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    if(pend - p < 2)
      throw std::runtime_error("");

    tag = get8(p);
    p += 1;
    length = get8(p);
    p += 1;

    *pp = p;
  }
};

struct descriptor
{
  descriptor_header header;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    header.unpack(pp, pend);
    *pp += header.length; //FIXME
  }
};


}


#endif
