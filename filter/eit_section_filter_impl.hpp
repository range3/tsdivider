#ifndef _TSSP_EIT_SECTION_FILTER_IMPL_HPP_
#define _TSSP_EIT_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "eit.hpp"

namespace tssp
{

void eit_section_filter::do_handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  //cout << "eit section filter" << endl;
  //hexdump(section_buffer, section_length, cout);
  event_information_table eit;
  eit.unpack(section_buffer, section_length);

  c.get_view().print(
      c.get_packet_num(),
      eit,
      subtable_is_changed(eit));

  // FIXME
  if(eit.header.section_number == eit.header.last_section_number) {
    version_
      [eit.header.table_id]
      [eit.header.transport_stream_id]
      [eit.transport_stream_id]
      [eit.original_network_id]
        = eit.header.version;
  }
}

inline
bool eit_section_filter::subtable_is_changed(
    const event_information_table& eit) {
  auto i = version_.find(
      eit.header.table_id);
  if(i == version_.end())
    return true;

  auto j = i->second.find(
      eit.header.transport_stream_id);
  if(j == i->second.end())
    return true;

  auto k = j->second.find(
      eit.transport_stream_id);
  if(k == j->second.end())
    return true;

  auto l = k->second.find(
      eit.original_network_id);

  return 
    l == k->second.end() ||
    l->second != eit.header.version;
}

}

#endif
