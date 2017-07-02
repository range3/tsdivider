#ifndef _TSD_CDT_SECTION_FILTER_IMPL_HPP_
#define _TSD_CDT_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "section.hpp"
#include "cdt.hpp"

namespace tsd
{

void cdt_section_filter::do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length) {
  section s;
  s.unpack(section_buffer, section_length);
  common_data_table cdt;
  s.convert<common_data_table>(cdt);

  if(!s.header.current_next_indicator)
    return;

  if(!subtable_is_changed(s.header, cdt))
    return;

  version_
    [s.header.table_id]
    [cdt.original_network_id]
      = s.header.version;
}

inline
bool cdt_section_filter::subtable_is_changed(
    const section_header& header,
    const common_data_table& cdt) {
  auto i = version_.find(header.table_id);
  if(i == version_.end())
    return true;

  auto j = i->second.find(
      cdt.original_network_id);

  return
    j == i->second.end() ||
    j->second != header.version;
}

}


#endif
