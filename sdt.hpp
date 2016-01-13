#ifndef _TSSP_SDT_HPP_
#define _TSSP_SDT_HPP_

#include "util.hpp"
#include "descriptor.hpp"

namespace tssp
{


struct service_description_table
{
  struct service
  {
    uint16_t service_id;
    std::vector<descriptor> descriptors;
  };

  // Table ID 0x42 (self stream) 0x46 (other stream)
  uint16_t original_network_id;
  std::vector<service> services;

  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    services.clear();

    original_network_id = get16(p);
    p += 3;

    while(pend - p > 4) {
      services.resize(services.size()+1);
      auto& s = services.back();

      s.service_id = get16(p);
      p += 3;
      size_t loop_length = get16(p) & 0x0FFF;
      p += 2;
      const uint8_t* ploop_end = p + loop_length;
      if(pend - p - 4 < loop_length)
        std::runtime_error("");

      while(p < ploop_end) {
        s.descriptors.resize(s.descriptors.size()+1);
        s.descriptors.back().unpack(&p, ploop_end);
      }
    }
  }
};

}


#endif
