#ifndef _TSSP_CONTEXT_HPP_
#define _TSSP_CONTEXT_HPP_

#include <memory>
#include <map>
#include <string>
#include "ts_reader.hpp"
#include "util.hpp"
#include "crc.hpp"

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

private:
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
          handle_section(c, section_buffer_);
        }
      }
    }
  }

protected:
  void handle_section(
      context& c,
      const std::string& section_buffer) {
    //DEBUG
    cerr << "-----section dump-----" << endl;
    tssp::hexdump(section_buffer.data(), section_buffer.size(), std::cerr);
    do_handle_section(c, section_buffer);
  }

  virtual void do_handle_section(
      context& c,
      const std::string& section_buffer) {}

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
          f->write_section_data(*this, pp, p.end() - pp, true);
        }
      }
      else {
        if(ci_ok) {
          f->write_section_data(*this, pp, p.end() - pp, false);
        }
      }
    }
    else {
      // not a section filter
    }
  }

private:
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

private:
  std::map<
    uint16_t,
    std::unique_ptr<filter> > pids_;
};
}


#endif
