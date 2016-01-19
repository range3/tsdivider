#ifndef _TSSP_TRANSPORT_PACKET_HPP_
#define _TSSP_TRANSPORT_PACKET_HPP_

#include "util.hpp"

namespace tssp
{

class transport_packet
{
public:
struct adaptation_field
{
  uint8_t length;
  uint8_t header_block;
  uint64_t pcr_base;
  uint16_t pcr_extension;
  std::string other_data_block; // FIXME: not parse yet

  adaptation_field() :
    length(0) {}

  uint8_t discontinuity_indicator() const {
    return header_block & 0x80;
  }

  uint8_t random_access_indicator() const {
    return header_block & 0x40;
  }

  uint8_t elementary_stream_priority_indicator() const {
    return header_block & 0x20;
  }

  uint8_t pcr_flag() const {
    return header_block & 0x10;
  }

  uint8_t opcr_flag() const {
    return header_block & 0x08;
  }

  uint8_t splicing_point_flag() const {
    return header_block & 0x04;
  }

  uint8_t transport_private_data_flag() const {
    return header_block & 0x02;
  }

  uint8_t adaptation_field_extension_flag() const {
    return header_block & 0x01;
  }

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;

    if(pend - p < 1)
      std::runtime_error("");

    length = get8(p);
    p += 1;

    if(length > 0) {
      const uint8_t* pafield_end = p + length;
      header_block = get8(p);
      p += 1;

      if(pcr_flag()) {
        pcr_base = (get32(p) << 1) | (get8(p+4) >> 7);
        p += 4;
        pcr_extension = ((get8(p) & 0x01) << 8) | get8(p+1);
        p += 2;
      }

      //FIXME: skip parsing other params
      other_data_block.assign(
          reinterpret_cast<const char*>(p),
          pafield_end - p);

      p = pafield_end;
    }

    *pp = p;
  }
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
      afield.unpack(&p, pend);
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
