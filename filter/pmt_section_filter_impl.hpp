#ifndef _TSSP_PMT_SECTION_FILTER_IMPL_HPP_
#define _TSSP_PMT_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "pmt.hpp"

namespace tssp
{

void pmt_section_filter::do_handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  program_map_table pmt;
  pmt.unpack(section_buffer, section_length);

  c.get_view().print(pmt,
      last_version_ != pmt.header.version);

  last_version_ = pmt.header.version;
}

}


#endif
