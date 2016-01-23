#ifndef _TSSP_DESCRIPTOR_BASE_HPP_
#define _TSSP_DESCRIPTOR_BASE_HPP_

namespace tssp
{

struct descriptor
{
  uint8_t tag;
  uint8_t length;
  const uint8_t* pdata;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    if(pend - p < 2)
      throw std::runtime_error("");

    tag = get8(p);
    p += 1;
    length = get8(p);
    p += 1;
    pdata = p;
    p += length;

    *pp = p;
  }

  template<typename T>
  T as() const {
    T t;
    convert(t);
    return std::move(t);
  }

  template<typename T>
  void convert(T& t) const {
    const uint8_t* p = pdata;
    t.unpack(&p, p+length);
  }
};


}


#endif
