#ifndef _TSD_CONTEXT_IMPL_HPP_
#define _TSD_CONTEXT_IMPL_HPP_

#include "filter.hpp"

namespace tsd {

context::context(std::unique_ptr<view> view) :
  view_(std::move(view)),
  packet_counter_(0)
{
  set_initial_filters();
}

inline
void context::set_initial_filters() {
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
  open_section_filter(
      0x0029, std::unique_ptr<section_filter>(
        new cdt_section_filter()));
}

inline
void context::clear() {
  pids_.clear();
  ts_trimmer_.reset();
  packet_counter_ = 0;
  transport_stream_id = boost::none;
  pat = boost::none;
  program_pcr.clear();
  first_pcr = boost::none;
  latest_pcr = boost::none;
  baseline_pcr = boost::none;
  baseline_time = boost::none;
  service_descriptors.clear();

  set_initial_filters();
}

inline
void context::handle_packet(const transport_packet& packet) {
  if(ts_trimmer_)
    ts_trimmer_->add_packet(packet);

  auto i_filter = pids_.find(packet.pid);
  if(i_filter != pids_.end()) {
    // find a correspondent filter 
    auto& f = i_filter->second;
    f->handle_packet(*this, packet);
  }

  packet_counter_ += 1;
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
void context::open_pcr_filter(uint16_t pid) {
  auto i = pids_.lower_bound(pid);
  if(i != pids_.end() && !(pid < i->first)) {
    return;
  }
  else {
    pids_.emplace_hint(
        i, pid, std::unique_ptr<pcr_filter>(new pcr_filter()));
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
