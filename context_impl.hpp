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
void context::handle_packet(const transport_packet& packet) {
  packet_counter_ += 1;
  auto i_filter = pids_.find(packet.pid);
  if(i_filter == pids_.end())
    return;

  // find a correspondent filter 
  auto& f = i_filter->second;

  // checking continuity
  uint8_t expect_cc;
  if(packet.has_payload())
    expect_cc = (f->last_ci()+1) & 0x0f;
  else
    expect_cc = f->last_ci();

  bool cc_ok =
    packet.pid == 0x1FFF || // null packet PID
                            // FIXME: discontinuity
    expect_cc == packet.continuity_counter;

  f->set_last_ci(packet.continuity_counter);

  if(f->is_section_filter()) {
    auto p = packet.payload;
    auto pend = packet.payload + packet.payload_size();
    if(packet.payload_unit_start_indicator) {
      // pointer field present
      auto pf = packet.pointer_field();
      p += 1;
      if(pf > pend - p)
        return;

      if(pf > 0 && cc_ok) {
        // write remaining section bytes
        f->write_data(*this, p, pf, false);
      }

      p += pf;
      if(p < pend) {
        f->write_data(*this, p, pend - p, true);
      }
    }
    else {
      if(cc_ok) {
        f->write_data(*this, p, pend - p, false);
      }
    }
  }
  else {
    // pes filter
    f->write_data(
        *this,
        packet.payload,
        packet.payload_size(),
        packet.payload_unit_start_indicator);
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
void context::open_pes_filter(
    uint16_t pid,
    std::unique_ptr<pes_filter> f) {
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
