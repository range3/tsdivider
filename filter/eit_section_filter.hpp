#ifndef _TSSP_EIT_SECTION_FILTER_HPP_
#define _TSSP_EIT_SECTION_FILTER_HPP_

#include "eit.hpp"

namespace tssp
{
class context;

// Event Information Table (EIT) section
// PID 0x0012
// Table ID 0x4E (self stream)
// Table ID 0x4F (other stream)
// Table ID 0x50 - 0x5F (self stream, event schedule)
// Table ID 0x60 - 0x6F (other stream, event schedule)
class eit_section_filter : public section_filter
{
public:
  eit_section_filter() :
    section_filter(true)
  {}
  virtual ~eit_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);

private:
  bool subtable_is_changed(
    const event_information_table& eit);

private:
  std::map<uint8_t,         // table_id
    std::map<uint16_t,      // table_id_extension
      std::map<uint16_t,    // transport_stream_id
        std::map<uint16_t,  // original_network_id
          int> > > >version_;
};

}

#include "filter/eit_section_filter_impl.hpp"

#endif
