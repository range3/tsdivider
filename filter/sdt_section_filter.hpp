#ifndef _TSSP_SDT_SECTION_FILTER_HPP_
#define _TSSP_SDT_SECTION_FILTER_HPP_

#include "sdt.hpp"
#include "aribstr.h"

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
    char tmpbuf[4096];
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
        cerr << "\t\t" << "tag : " << (int)d->tag << endl;
        cerr << "\t\t" << "length : " << (int)d->length << endl;
        if(d->tag == service_descriptor::TAG) {
          auto sd = d->as<service_descriptor>();
          cerr << "\t\t" << "service_type: " << (int)sd->service_type << endl;
          AribToString(
              tmpbuf,
              sd->service_provider_name.data(),
              sd->service_provider_name.size());
          cerr << "\t\t" << "service_provider_name: " << tmpbuf << endl;
          AribToString(
              tmpbuf,
              sd->service_name.data(),
              sd->service_name.size());
          cerr << "\t\t" << "service_name: " << tmpbuf << endl;
        }
      }
    }
  }
};

}

#endif
