#ifndef _TSSP_TABLE_HEADER_HPP_
#define _TSSP_TABLE_HEADER_HPP_

namespace tssp
{

template<typename MixIn>
class table_header
{
public:
  uint8_t tid() const {
    return *(payload()+1);
  }

  uint16_t section_length() const {
    return ((*(payload()+2) & 0x0F) << 8) + *(payload()+3);
  }

protected:
  const uint8_t* payload() const {
    return reinterpret_cast<const uint8_t*>(
        static_cast<const MixIn*>(this)->get_packet().payload());
  }
};

}


#endif
