#ifndef _TSSP_EIT_HPP_
#define _TSSP_EIT_HPP_

#include "section_header.hpp"
#include "util.hpp"
#include "descriptor.hpp"
#include "aribtime.hpp"

namespace tssp
{


struct event_information_table
{
  struct event
  {
    uint16_t event_id;
    aribtime start_time;
    aribduration duration;
    uint8_t running_status;
    uint8_t free_ca_mode;
    std::vector<descriptor> descriptors;
  };

  section_header header;
  uint16_t transport_stream_id;
  uint16_t original_network_id;
  uint8_t segment_last_section_number;
  uint8_t last_table_id;
  std::vector<event> events;

  void unpack(const char* data, size_t size) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
    const uint8_t* pend = p+size;

    events.clear();

    header.unpack(&p, pend);

    transport_stream_id = get16(p);
    p += 2;
    original_network_id = get16(p);
    p += 2;
    segment_last_section_number = get8(p);
    p += 1;
    last_table_id = get8(p);
    p += 1;

    while(pend - p > 4) {
      events.resize(events.size()+1);
      auto& e = events.back();

      e.event_id = get16(p);
      p += 2;
      e.start_time.unpack(&p, pend);
      e.duration.unpack(&p, pend);
      e.running_status = get8(p) >> 5;
      e.free_ca_mode = (get8(p) >> 4) & 0x01;
      size_t loop_length = get16(p) & 0x0FFF;
      p += 2;
      const uint8_t* ploop_end = p + loop_length;
      if(pend - p - 4 < loop_length)
        std::runtime_error("");

      while(p < ploop_end) {
        e.descriptors.resize(e.descriptors.size()+1);
        e.descriptors.back().unpack(&p, ploop_end);
      }
    }
  }
};

}


#endif
