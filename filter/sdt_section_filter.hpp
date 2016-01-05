#ifndef _TSSP_SDT_SECTION_FILTER_HPP_
#define _TSSP_SDT_SECTION_FILTER_HPP_

#include "sdt.hpp"

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
      size_t section_length) {
    service_description_table sdt;
    sdt.unpack(section_buffer, section_length);

    cerr << "----- sdt section -----" << endl;
    cerr << "dump : " << endl;
    tssp::hexdump(section_buffer, section_length, std::cerr);
    cerr << "header : " << endl;
    dump_section_header(sdt.header);
    cerr << "services : " << endl;
    for(auto& s : sdt.services) {
      cerr << "\t" << "service_id : " << (int)s->service_id << endl;
      for(auto& d : s->descriptors) {
        cerr << "\t\t" << "tag : " << (int)d->header.tag << endl;
        cerr << "\t\t" << "length : " << (int)d->header.length << endl;
      }
    }
  }
};

}

#endif
