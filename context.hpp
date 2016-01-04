#ifndef _TSSP_CONTEXT_HPP_
#define _TSSP_CONTEXT_HPP_

#include <memory>
#include <map>
#include <string>
#include "ts_reader.hpp"

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
    filter()
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

  }
protected:
  std::string section_buffer_;
};

class pat_section_filter : public section_filter
{
public:
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
