#ifndef _TSSP_CONTEXT_HPP_
#define _TSSP_CONTEXT_HPP_

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "ts_reader.hpp"
#include "util.hpp"
#include "crc.hpp"
#include "pat.hpp"
#include "pmt.hpp"

namespace tssp
{
class context;

class filter
{
public:
  filter() :
    last_ci_(0xf0)
  {}
  virtual ~filter() {}
  virtual bool is_section_filter() const {
    return false;
  }
  virtual void write_section_data(
      context& c,
      const char* data,
      size_t size,
      bool is_start) = 0;


  uint8_t last_ci() const {
    return last_ci_;
  }

  void set_last_ci(uint8_t ci) {
    last_ci_ = ci;
  }

protected:
  uint8_t last_ci_;
};

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
    cerr << "-----section dump-----" << endl;
    tssp::hexdump(section_buffer, section_length, std::cerr);
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

class pat_section_filter : public section_filter
{
public:
  pat_section_filter() :
    section_filter(true)
  {}
  virtual ~pat_section_filter() {}

protected:
  virtual void do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length);
};


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

    cerr << "----- pmt -----" << endl;
    dump_section_header(pmt.header);
    cerr << "pcr_pid : " << (int)pmt.pcr_pid << endl;
    cerr << "program_info(descriptor)" << endl;
    {
      for(auto& i : pmt.program_info) {
        cerr << "\t" << "tag : " << (int)i->header.tag << endl;
        cerr << "\t" << "length : " << (int)i->header.length << endl;
      }
    }
    cerr << "program_elements" << endl;
    {
      for(auto& i : pmt.program_elements) {
        cerr << "\t" << "stream_type : " << (int)i->stream_type << endl;
        cerr << "\t" << "elementary_pid : " << (int)i->elementary_pid << endl;
        cerr << "\t" << "es_info(descriptor)" << endl;
        for(auto& j : i->es_info) {
          cerr << "\t\t" << "tag : " << (int)j->header.tag << endl;
          cerr << "\t\t" << "length : " << (int)j->header.length << endl;

        }
      }
    }
  }

};

class context
{
public:
  context() {
    open_section_filter(
        0, std::unique_ptr<section_filter>(
          new pat_section_filter()));
  }

  void handle_packet(const packet& p) {

    auto i_filter = pids_.find(p.pid());
    if(i_filter == pids_.end())
      return;

    // find a correspondent filter 
    auto& f = i_filter->second;

    // DEBUG: hexdump
    cerr << "-----PID " << p.pid() << "-----" << endl;
    p.hexdump();

    // checking continuity
    uint8_t expect_ci = p.has_payload() ? (f->last_ci()+1) & 0x0f : f->last_ci();
    bool ci_ok =
      p.pid() == 0x1FFF || // null packet PID
                           // FIXME: discontinuity
      expect_ci == p.continuity_index();

    f->set_last_ci(p.continuity_index());

    if(f->is_section_filter()) {
      auto pp = p.payload();
      if(p.payload_uint_start_indicator()) {
        // pointer field present
        auto pf = p.pointer_field();
        pp += 1;
        if(pf > p.end() - pp)
          return;

        if(pf > 0 && ci_ok) {
          // write remaining section bytes
          f->write_section_data(*this, pp, pf, false);
        }

        pp += pf;
        if(pp < p.end()) {
          cerr << "aa" << endl;
          f->write_section_data(*this, pp, p.end() - pp, true);
        }
      }
      else {
        cerr << "bb" << endl;
        if(ci_ok) {
          cerr << "cc" << endl;
          f->write_section_data(*this, pp, p.end() - pp, false);
        }
      }
    }
    else {
      // not a section filter
    }
  }

  void open_section_filter(
      uint16_t pid,
      std::unique_ptr<section_filter>&& f) {
    auto i = pids_.lower_bound(pid);
    if(i != pids_.end() && !(pid < i->first)) {
      return;
    }
    else {
      pids_.emplace_hint(i, pid, std::move(f));
    }
  }

  bool is_opened(uint16_t pid) const {
    return pids_.count(pid);
  }

private:
  std::map<
    uint16_t,
    std::unique_ptr<filter> > pids_;
};


// impl

void pat_section_filter::do_handle_section(
      context& c,
      const char* section_buffer,
      size_t section_length) {
  program_association_table pat;
  pat.unpack(section_buffer, section_length);

  //DEBUG
  cerr << "----- pat -----" << endl;
  dump_section_header(pat.header);
  {
    cerr << "program number | pmt pid" << endl;
    auto i = pat.program_num_to_pid.begin();
    auto i_end = pat.program_num_to_pid.end();
    for(; i != i_end; ++i) {
      cerr << (int)i->first << " | " << (int)i->second << endl;
      if(i->first != 0) {
        if(!c.is_opened(i->second)) {
          c.open_section_filter(
              i->second, std::unique_ptr<section_filter>(
                new pmt_section_filter()));
        }
      }
    }
  }
}

}

#endif
