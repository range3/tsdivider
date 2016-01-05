#ifndef _TSSP_TOT_SECTION_FILTER_HPP_
#define _TSSP_TOT_SECTION_FILTER_HPP_

#include <chrono>
#include "tot.hpp"

namespace tssp
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
      size_t section_length) {
    time_offset_table tot;
    tot.unpack(section_buffer, section_length);

    cerr << "----- tot section -----" << endl;
    cerr << "dump : " << endl;
    tssp::hexdump(section_buffer, section_length, std::cerr);
    cerr << "table id : " << (int)tot.table_id << endl;
    cerr << dec;
    time_t t = std::chrono::system_clock::to_time_t(tot.time());
    cerr << "time : " << std::ctime(&t);
  }
};

}

#endif
