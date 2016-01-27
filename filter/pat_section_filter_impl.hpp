#ifndef _TSD_PAT_SECTION_FILTER_IMPL_HPP_
#define _TSD_PAT_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "section.hpp"
#include "pat.hpp"
#include "filter/pmt_section_filter.hpp"

namespace tsd
{

void pat_section_filter::do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length) {
  section s;
  s.unpack(section_buffer, section_length);
  program_association_table pat;
  s.convert<program_association_table>(pat);

  c.get_view().print(
      c.get_packet_num(),
      s.header,
      pat,
      last_version_ != s.header.version);

  if(last_version_ == s.header.version)
    return;

  c.pat = pat;

  for(auto& i : pat.association) {
    if(i.program_number != 0) {
      if(!c.is_opened(i.pmt_pid)) {
        c.open_section_filter(
            i.pmt_pid,
            std::unique_ptr<section_filter>(
              new pmt_section_filter()));
      }
    }
  }

  last_version_ = s.header.version;
}

}


#endif
