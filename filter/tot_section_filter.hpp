#ifndef _TSD_TOT_SECTION_FILTER_HPP_
#define _TSD_TOT_SECTION_FILTER_HPP_

namespace tsd
{
class context;

// time offset section (or time data section)
// tot and tdt are assigned to the same pid (0x0014)
class tot_section_filter : public section_filter
{
public:
  tot_section_filter() :
    section_filter(false)
  {}
  virtual ~tot_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);
};

}

#include "filter/tot_section_filter_impl.hpp"

#endif
