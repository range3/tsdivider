#ifndef _TSSP_TOT_SECTION_FILTER_IMPL_HPP_
#define _TSSP_TOT_SECTION_FILTER_IMPL_HPP_

#include <chrono>
#include "context.hpp"
#include "tot.hpp"

namespace tssp
{

void tot_section_filter::do_handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  time_offset_table tot;
  tot.unpack(section_buffer, section_length);

  c.get_view().print(
      c.get_packet_num(),
      tot);
}

}

#endif
