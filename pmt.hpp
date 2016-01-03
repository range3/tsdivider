#ifndef _TSSP_PMT_HPP_
#define _TSSP_PMT_HPP_

#include "crc.hpp"

namespace tssp
{
class descriptor
{
public:
  descriptor(
      const char* head,
      size_t remain_buff_size) :
    head_(head),
    remain_(remain_buff_size)
  {}

  uint8_t tag() const {
    return *reinterpret_cast<const uint8_t*>(head_);
  }

  uint8_t length() const {
    return *reinterpret_cast<const uint8_t*>(head_+1);
  }

  const char* payload() const {
    return head_+2;
  }

  bool has_next() const {
    return remain_ >= 2+2+length();
  }

  const descriptor next() const {
    return descriptor(payload()+length(), remain_-2-length());
  }

private:
  const char* head_;
  size_t remain_;
};

class program_element
{
public:
  program_element(
      const char* head,
      size_t remain_buff_size) :
    head_(head),
    remain_(remain_buff_size)
  {}

  uint8_t stream_type() const {
    return *reinterpret_cast<const uint8_t*>(head_);
  }

  uint16_t elementary_pid() const {
    return
      ((*reinterpret_cast<const uint8_t*>(head_+1) & 0x1F) << 8) + 
      *reinterpret_cast<const uint8_t*>(head_+2);
  }

  uint16_t es_info_length() const {
    return
      ((*reinterpret_cast<const uint8_t*>(head_+3) & 0x0F) << 8) +
      *reinterpret_cast<const uint8_t*>(head_+4);
  }

  const descriptor es_info() const {
    return descriptor(head_+5, es_info_length());
  }

  bool has_next() const {
    return remain_ >= 5+5+es_info_length();
  }

  const program_element next() const {
    return program_element(
        head_+5+es_info_length(),
        remain_-5-es_info_length());
  }

private:
  const char* head_;
  size_t remain_;
};

class pmt : public table_header<pmt>
{
public:
  pmt(const packet& packet) :
    packet_(packet)
  {}

  const packet& get_packet() const {
    return packet_;
  }

  uint16_t program_number() const {
    return (*(payload()+4)<<8) + *(payload()+5);
  }

  uint8_t version_number() const {
    return (*(payload()+6) & 0x3F) >> 1;
  }

  uint8_t current_next_indicator() const {
    return (*(payload()+6) & 0x01);
  }

  uint8_t section_number() const {
    return *(payload()+7);
  }

  uint8_t last_section_number() const {
    return *(payload()+8);
  }

  uint16_t pcr_pid() const {
    return ((*(payload()+9) & 0x1F) << 8) + *(payload()+10);
  }

  // the number of bytes of the descriptors immediately following this field
  uint16_t program_info_length() const {
    return ((*(payload()+11) & 0x0F) << 8) + *(payload()+12);
  }

  const descriptor program_info() const {
    return descriptor(
        packet_.payload()+13,
        program_info_length());
  }

  uint16_t program_element_length() const {
    return section_length()-9-program_info_length()-4;
  }

  const program_element get_first_program_element() const {
    return program_element(
        packet_.payload()+13+program_info_length(),
        section_length()-9-program_info_length()-4);
  }

  uint32_t crc32() const {
    const uint8_t* h = payload()+4+section_length()-4;
    return (*h << 24) | (*(h+1) << 16) | (*(h+2) << 8) | *(h+3);
  }

  bool check_crc32() const {
    crc32_ts crc_calc;
    crc_calc.process_bytes(
        payload()+1, 3+section_length()-4);
    return crc_calc() == crc32();
  }

private:
  const packet& packet_;
};

}



#endif
