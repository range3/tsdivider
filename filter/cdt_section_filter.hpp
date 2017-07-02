#ifndef _TSD_CDT_SECTION_FILTER_HPP_
#define _TSD_CDT_SECTION_FILTER_HPP_

#include "section_header.hpp"
#include "cdt.hpp"

namespace tsd
{
class context;

class cdt_section_filter : public section_filter
{
public:
  cdt_section_filter() :
    section_filter(true)
  {}
  virtual ~cdt_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);

private:
  bool subtable_is_changed(
      const section_header& header,
      const common_data_table& cdt);

private:
  std::map<uint8_t,     // table_id
    std::map<uint16_t,  // original_network_id
      int> > version_;
};

}

#include "filter/cdt_section_filter_impl.hpp"

#endif
