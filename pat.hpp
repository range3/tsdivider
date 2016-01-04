#ifndef _TSSP_PAT_HPP_
#define _TSSP_PAT_HPP_

#include <cstdint>
#include <map>


#include "section_header.hpp"
#include "util.hpp"


namespace tssp
{
class program_association_table
{
public:
  section_header header;
  std::map<uint16_t, uint16_t> program_num_to_pid;

public:
  program_association_table() {}

  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    header.unpack(&p, pend);

    while(pend - p >= 4+4) {
      uint16_t pnum = get16(p);
      p += 2;
      uint16_t pmt_pid = get16(p) & 0x1FFF;
      p += 2;
      program_num_to_pid[pnum] = pmt_pid;
    }
  }
};

}

#endif
