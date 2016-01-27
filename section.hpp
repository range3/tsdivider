#ifndef _TSD_SECTION_HPP_
#define _TSD_SECTION_HPP_

#include "section_header.hpp"

namespace tsd
{
class section
{
public:
  section_header header;
  const char* payload;

public:
  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    unpack(&p, p+size);
  }

  void unpack(
      const uint8_t** pp,
      const uint8_t* pend) {
    const uint8_t* p = *pp;

    header.unpack(&p, pend);
    payload = reinterpret_cast<const char*>(p);

    *pp = p;
  }

  template<typename T>
  std::unique_ptr<T> as() const {
    std::unique_ptr<T> d(new T);
    convert(*d);
    return std::move(d);
  }

  template<typename T>
  void convert(T& t) const {
    t.unpack(payload, header.get_payload_length());
  }


};

}

#endif
