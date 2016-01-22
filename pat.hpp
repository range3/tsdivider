#ifndef _TSSP_PAT_HPP_
#define _TSSP_PAT_HPP_

#include <cstdint>
#include <map>

#include "util.hpp"

namespace tssp
{
class program_association_table
{
public:
  struct association_pair
  {
    association_pair(uint16_t pnum, uint16_t pmt) :
      program_number(pnum),
      pmt_pid(pmt) {}

    uint16_t program_number;
    uint16_t pmt_pid;
  };
public:
  std::vector<association_pair> association;

public:
  program_association_table() {}

  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    while(pend - p >= 4+4) {
      association.push_back(
          association_pair(get16(p), get16(p+2) & 0x1FFF));
      p += 4;
    }
  }
};

}

#endif
