#ifndef _TSD_CDT_HPP_
#define _TSD_CDT_HPP_

#include <cstdint>
#include <vector>

#include "util.hpp"
#include "descriptor.hpp"

namespace tsd
{
class common_data_table
{
public:
  uint16_t original_network_id;
  uint8_t  data_type;
  std::vector<descriptor> descriptors;
  const uint8_t* data_module_byte;
  size_t data_module_byte_length;

public:
  common_data_table() {}

  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    original_network_id = get16(p);
    p += 2;
    data_type = get8(p);
    p += 1;
    uint16_t loop_length = get16(p) & 0x0FFF;
    p += 2;

    const uint8_t* ploop_end = p + loop_length;
    if(pend - p - 4 < loop_length)
      std::runtime_error("");

    while(p < ploop_end) {
      descriptors.resize(descriptors.size()+1);
      descriptors.back().unpack(&p, ploop_end);
    }

    data_module_byte = p;
    data_module_byte_length = pend - 4 - p;
  }
};

}

#endif
