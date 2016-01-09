#ifndef _TSSP_SDT_SECTION_FILTER_HPP_
#define _TSSP_SDT_SECTION_FILTER_HPP_

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
};

}

#include "filter/sdt_section_filter_impl.hpp"

#endif
