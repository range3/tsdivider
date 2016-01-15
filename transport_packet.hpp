#ifndef _TSSP_TRANSPORT_PACKET_HPP_
#define _TSSP_TRANSPORT_PACKET_HPP_

#include "util.hpp"

namespace tssp
{

class transport_packet
{
public:
struct adaptation_field  //FIXME: not impl yet
{
  uint8_t length;

  adaptation_field() :
    length(0) {}
};

public:
  static constexpr uint8_t sync_byte = 0x47;
  static constexpr size_t size = 188;

public:
  uint8_t transport_error_indicator;
  uint8_t payload_unit_start_indicator;
  uint8_t transport_priority;
  uint16_t pid;
  uint8_t transport_scrambling_control;
  uint8_t adaptation_field_control;
  uint8_t continuity_counter;
  adaptation_field afield;
  const char* payload;

public:
  void unpack(const char* data, size_t size) {
    auto p = reinterpret_cast<const uint8_t*>(data);
    unpack(&p, p+size);
  }

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    auto p = *pp;
    if(get8(p) != sync_byte)
      std::runtime_error("syncbyte error");
    p += 1;
    transport_error_indicator = (get8(p) & 0x80) >> 7;
    payload_unit_start_indicator = (get8(p) & 0x40) >> 6;
    transport_priority = (get8(p) & 0x20) >> 5;
    pid = get16(p) & 0x1FFF;
    p += 2;
    transport_scrambling_control = (get8(p) & 0x60) >> 6;
    adaptation_field_control = (get8(p) & 0x30) >> 4;
    continuity_counter = get8(p) & 0x0F;
    p += 1;
    if(has_adaptation_field()) {
      afield.length = get8(p);
      p += 1 + afield.length;
    }
    if(has_payload())
      payload = reinterpret_cast<const char*>(p);
    else
      payload = nullptr;

    *pp = p;
  }

  size_t pointer_field() const {
    return *reinterpret_cast<const uint8_t*>(payload);
  }
  size_t payload_size() const {
    if(has_adaptation_field())
      return size - 5 - afield.length;
    else
      return size - 4;
  }

  bool has_payload() const {
    return adaptation_field_control & 0x01;
  }
  bool has_adaptation_field() const {
    return adaptation_field_control & 0x02;
  }
};


}


#endif
