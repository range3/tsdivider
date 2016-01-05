#ifndef _TSSP_SDT_HPP_
#define _TSSP_SDT_HPP_

#include "section_header.hpp"
#include "util.hpp"
#include "pmt.hpp" //FIXME: for descriptor

namespace tssp
{


struct service_description_table
{
  struct service
  {
    uint16_t service_id;
    std::vector<std::unique_ptr<descriptor> > descriptors;
  };

  // Table ID 0x42 (self stream) 0x46 (other stream)
  section_header header;
  uint16_t original_network_id;
  std::vector<std::unique_ptr<service> > services;

  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    header.unpack(&p, pend);

    original_network_id = get16(p);
    p += 3;

    while(pend - p > 4) {
      std::unique_ptr<service> s(new service);
      s->service_id = get16(p);
      p += 3;
      size_t loop_length = get16(p) & 0x0FFF;
      p += 2;
      const uint8_t* ploop_end = p + loop_length;
      if(pend - p - 4 < loop_length)
        std::runtime_error("");

      while(p < ploop_end) {
        std::unique_ptr<descriptor> d(new descriptor);
        d->unpack(&p, ploop_end);
        s->descriptors.push_back(std::move(d));
      }

      services.push_back(std::move(s));
    }
  }
};

}


#endif
