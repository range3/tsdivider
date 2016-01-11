#ifndef _TSSP_PAT_SECTION_FILTER_HPP_
#define _TSSP_PAT_SECTION_FILTER_HPP_


namespace tssp
{
class context;

class pat_section_filter : public section_filter
{
public:
  pat_section_filter() :
    section_filter(true),
    last_version_(-1)
  {}
  virtual ~pat_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);

private:
  int last_version_;
};

}

#include "filter/pat_section_filter_impl.hpp"

#endif
