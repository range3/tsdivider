#ifndef _TSSP_SDT_SECTION_FILTER_HPP_
#define _TSSP_SDT_SECTION_FILTER_HPP_

#include <tuple>

namespace tssp
{
class context;

// Service Description Table (SDT)
// PID 0x0011
// Table ID 0x42 (self stream) 0x46 (other stream)
class sdt_section_filter : public section_filter
{
public:
  sdt_section_filter() :
    section_filter(true)
  {}
  virtual ~sdt_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);

private:
  bool is_changed(
      const section_header& header,
      const service_description_table& sdt) const {
    auto i = version_.find(
        std::make_tuple(
          header.table_id,
          header.table_id_extension,
          sdt.original_network_id));
    return i == version_.end() || i->second != header.version;
  }

private:
  std::map<
    std::tuple<
      uint8_t,   // table_id
      uint16_t,  // table_id_extension
      uint16_t>, // original_network_id
    uint8_t> version_;
};

}

#include "filter/sdt_section_filter_impl.hpp"

#endif
