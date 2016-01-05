#ifndef _TSSP_CONTEXT_HPP_
#define _TSSP_CONTEXT_HPP_

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "ts_reader.hpp"
#include "util.hpp"
#include "crc.hpp"

#include "filter.hpp"

namespace tssp {

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

}

#include "filter/pat_section_filter_impl.hpp"

#endif
