#ifndef _TSSP_SDT_SECTION_FILTER_IMPL_HPP_
#define _TSSP_SDT_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "section.hpp"
#include "sdt.hpp"

namespace tssp
{

void sdt_section_filter::do_handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  section s;
  s.unpack(section_buffer, section_length);
  service_description_table sdt;
  s.convert(sdt);

  c.get_view().print(
      c.get_packet_num(),
      s.header,
      sdt,
      is_changed(
        s.header.table_id_extension,
        s.header.version));
  tsid_to_last_version_[s.header.table_id_extension] = 
    s.header.version;
}

}

#endif
