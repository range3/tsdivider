#ifndef _TSSP_PMT_SECTION_FILTER_HPP_
#define _TSSP_PMT_SECTION_FILTER_HPP_

namespace tssp
{
class context;

class pmt_section_filter : public section_filter
{
public:
  pmt_section_filter() :
    section_filter(true)
  {}
  virtual ~pmt_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);
};

}

#include "filter/pmt_section_filter_impl.hpp"


#endif
