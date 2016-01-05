#ifndef _TSSP_PMT_SECTION_FILTER_HPP_
#define _TSSP_PMT_SECTION_FILTER_HPP_

#include "pmt.hpp"

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
      size_t section_length) {
    program_map_table pmt;
    pmt.unpack(section_buffer, section_length);

    //cerr << "----- pmt -----" << endl;
    //dump_section_header(pmt.header);
    //cerr << "pcr_pid : " << (int)pmt.pcr_pid << endl;
    //cerr << "program_info(descriptor)" << endl;
    //{
    //  for(auto& i : pmt.program_info) {
    //    cerr << "\t" << "tag : " << (int)i->header.tag << endl;
    //    cerr << "\t" << "length : " << (int)i->header.length << endl;
    //  }
    //}
    //cerr << "program_elements" << endl;
    //{
    //  for(auto& i : pmt.program_elements) {
    //    cerr << "\t" << "stream_type : " << (int)i->stream_type << endl;
    //    cerr << "\t" << "elementary_pid : " << (int)i->elementary_pid << endl;
    //    cerr << "\t" << "es_info(descriptor)" << endl;
    //    for(auto& j : i->es_info) {
    //      cerr << "\t\t" << "tag : " << (int)j->header.tag << endl;
    //      cerr << "\t\t" << "length : " << (int)j->header.length << endl;
    //    }
    //  }
    //}
  }
};

}


#endif
