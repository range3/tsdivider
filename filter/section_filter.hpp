#ifndef _TSSP_SECTION_FILTER_HPP_
#define _TSSP_SECTION_FILTER_HPP_

namespace tssp
{
class context;

class section_filter : public filter
{
public:
  section_filter() :
    filter(),
    do_crc_check_(false),
    last_version_(-1)
  {}
  section_filter(
      bool do_crc_check) :
    filter(),
    do_crc_check_(do_crc_check)
  {}

  virtual ~section_filter() {}
  virtual bool is_section_filter() const {
    return true;
  }

  virtual void write_section_data(
      context& c,
      const char* data,
      size_t size,
      bool is_start);

protected:
  void handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);

  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length) {}

protected:
  std::string section_buffer_;
  bool do_crc_check_;
  int last_version_;
};
}

#include "filter/section_filter_impl.hpp"


#endif
