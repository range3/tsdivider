#ifndef _TSSP_TOT_SECTION_FILTER_IMPL_HPP_
#define _TSSP_TOT_SECTION_FILTER_IMPL_HPP_

#include <chrono>
#include "context.hpp"
#include "section.hpp"
#include "tot.hpp"

namespace tssp
{

void tot_section_filter::do_handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  section s;
  s.unpack(section_buffer, section_length);
  time_offset_table tot;
  s.convert(tot);

  c.get_view().print(
      c.get_packet_num(),
      s.header,
      tot);

  if(!c.baseline_time && c.latest_pcr) {
    c.baseline_time = tot.time.to_time_t();
    c.baseline_pcr  = *c.latest_pcr;
  }
}

}

#endif
