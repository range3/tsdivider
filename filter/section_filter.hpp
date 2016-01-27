#ifndef _TSD_SECTION_FILTER_HPP_
#define _TSD_SECTION_FILTER_HPP_


namespace tsd
{
class context;
class transport_packet;

class section_filter : public filter
{
public:
  section_filter() :
    filter(),
    do_crc_check_(false)
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

  virtual void handle_packet(
      context& c,
      const transport_packet& packet);

private:
  void write_data(
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
};
}

#include "filter/section_filter_impl.hpp"


#endif
