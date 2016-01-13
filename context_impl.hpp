#ifndef _TSSP_CONTEXT_IMPL_HPP_
#define _TSSP_CONTEXT_IMPL_HPP_

#include "filter.hpp"

namespace tssp {

context::context(std::unique_ptr<view> view) :
  view_(std::move(view)),
  packet_counter_(0)
{
  open_section_filter(
      0x0000, std::unique_ptr<section_filter>(
        new pat_section_filter()));
  open_section_filter(
      0x0011, std::unique_ptr<section_filter>(
        new sdt_section_filter()));
  open_section_filter(
      0x0012, std::unique_ptr<section_filter>(
        new eit_section_filter()));
  open_section_filter(
      0x0026, std::unique_ptr<section_filter>(
        new eit_section_filter()));
  open_section_filter(
      0x0027, std::unique_ptr<section_filter>(
        new eit_section_filter()));
  open_section_filter(
      0x0014, std::unique_ptr<section_filter>(
        new tot_section_filter()));
}

inline
void context::handle_packet(const packet& p) {
  packet_counter_ += 1;
  auto i_filter = pids_.find(p.pid());
  if(i_filter == pids_.end())
    return;

  // find a correspondent filter 
  auto& f = i_filter->second;

  // DEBUG: hexdump
  //if(p.pid() == 0x0014) {
  //  cerr << "-----PID " << p.pid() << "-----" << endl;
  //  p.hexdump();
  //}

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

inline
void context::open_section_filter(
    uint16_t pid,
    std::unique_ptr<section_filter> f) {
  auto i = pids_.lower_bound(pid);
  if(i != pids_.end() && !(pid < i->first)) {
    return;
  }
  else {
    pids_.emplace_hint(i, pid, std::move(f));
  }
}

inline
bool context::is_opened(uint16_t pid) const {
  return pids_.count(pid);
}

inline
view& context::get_view() {
  return *view_;
}

}


#endif
