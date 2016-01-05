#ifndef _TSSP_SECTION_FILTER_HPP_
#define _TSSP_SECTION_FILTER_HPP_

#include "section_header.hpp"

namespace tssp
{
class context;

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

  virtual void write_section_data(
      context& c,
      const char* data,
      size_t size,
      bool is_start) {
    if(is_start) {
      section_buffer_.assign(data, size);
    }
    else {
      section_buffer_.append(data, size);
    }

    if(section_buffer_.size() >= 3) {
      uint16_t section_length =
        (get16(section_buffer_.data()+1) & 0x0FFF) + 3;
      if(section_length > 4096) {
        return; //too long
      }
      if(section_buffer_.size() >= section_length) {
        bool crc_valid = true;
        if(do_crc_check_) {
          if(section_length < 4)
            return;
          uint32_t crc32 = get32(section_buffer_.data()+section_length-4);
          crc32_ts crc_calc;
          crc_calc.process_bytes(
              section_buffer_.data(),
              section_length-4);
          crc_valid = (crc32 == crc_calc());
        }

        if(crc_valid) {
          handle_section(c, section_buffer_.data(), section_length);
        }
      }
    }
  }

protected:
  void handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length) {
    //DEBUG
    //cerr << "-----section dump-----" << endl;
    //tssp::hexdump(section_buffer, section_length, std::cerr);
    do_handle_section(c, section_buffer, section_length);
  }

  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length) {}


  // DEBUG
  void dump_section_header(const section_header& sh) const {
    cerr << "table id : " << (int)sh.table_id << endl;
    cerr << "tranport stream id : " << (int)sh.transport_stream_id << endl;
    cerr << "version : " << (int)sh.version << endl;
    cerr << "section number : " << (int)sh.section_number << endl;
    cerr << "last section number : " << (int)sh.last_section_number << endl;
  }

protected:
  std::string section_buffer_;
  bool do_crc_check_;
};
}


#endif
