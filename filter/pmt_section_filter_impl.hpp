#ifndef _TSSP_PMT_SECTION_FILTER_IMPL_HPP_
#define _TSSP_PMT_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "section.hpp"
#include "pmt.hpp"

namespace tssp
{

void pmt_section_filter::do_handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  section s;
  s.unpack(section_buffer, section_length);
  program_map_table pmt;
  s.convert(pmt);

  c.get_view().print(
      c.get_packet_num(),
      s.header,
      pmt,
      last_version_ != s.header.version);

  last_version_ = s.header.version;
}

}


#endif
