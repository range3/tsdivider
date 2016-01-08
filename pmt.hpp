#ifndef _TSSP_PMT_HPP_
#define _TSSP_PMT_HPP_

#include <vector>
#include <memory>

#include "section_header.hpp"
#include "util.hpp"
#include "descriptor.hpp"

namespace tssp
{
struct program_element
{
  uint8_t stream_type;
  uint16_t elementary_pid;
  std::vector<descriptor> es_info;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    if(pend - p < 5)
      std::runtime_error("");

    stream_type = get8(p);
    p += 1;
    elementary_pid = get16(p) & 0x1FFF;
    p += 2;
    int32_t es_info_length = get16(p) & 0x0FFF;
    p += 2;
    const uint8_t* peiend = p + es_info_length;
    es_info.clear();
    while(peiend - p >= 2) {
      es_info.resize(es_info.size()+1);
      es_info.back().unpack(&p, pend);
    }
    p = peiend;
    *pp = p;
  }
};

struct program_map_table
{
  section_header header;
  uint16_t pcr_pid;
  std::vector<descriptor> program_info;
  std::vector<program_element> program_elements;

public:
  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    header.unpack(&p, pend);

    pcr_pid = get16(p) & 0x1FFF;
    p += 2;
    int32_t program_info_length = get16(p) & 0x0FFF;
    p += 2;
    const uint8_t* piend = p + program_info_length;
    program_info.clear();
    while(piend - p >= 2) {
      program_info.resize(program_info.size()+1);
      program_info.back().unpack(&p, pend);
    }
    p = piend;

    program_elements.clear();
    while(pend - p >= 5+4) {
      program_elements.resize(program_elements.size()+1);
      program_elements.back().unpack(&p, pend);
    }
  }

};

}



#endif
