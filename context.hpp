#ifndef _TSD_CONTEXT_HPP_
#define _TSD_CONTEXT_HPP_

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "util.hpp"
#include "crc.hpp"

#include "transport_packet.hpp"
#include "view.hpp"
#include "ts_trimmer.hpp"

#include "boost/optional.hpp"

namespace tsd {

class filter;
class section_filter;
class pes_filter;

class context
{
public:
  context(std::unique_ptr<view> view);

  void handle_packet(const transport_packet& p);

  void open_section_filter(
      uint16_t pid,
      std::unique_ptr<section_filter> f);
  void open_pes_filter(
      uint16_t pid,
      std::unique_ptr<pes_filter> f);
  void open_pcr_filter(uint16_t pid);

  bool is_opened(uint16_t pid) const;

  view& get_view();

  uint64_t get_packet_num() const {
    return packet_counter_;
  }

  void set_ts_trimmer(std::unique_ptr<ts_trimmer> t) {
    ts_trimmer_ = std::move(t);
  }
  void signal_pcr(uint64_t pcr) {
    if(ts_trimmer_)
      ts_trimmer_->signal_pcr(pcr);
  }
  void signal_pmt() {
    if(ts_trimmer_)
      ts_trimmer_->signal_pmt();
  }
  void signal_eit() {
    if(ts_trimmer_)
      ts_trimmer_->signal_eit();
  }

private:
  std::map<
    uint16_t,
    std::unique_ptr<filter> > pids_;

  std::unique_ptr<view> view_;
  std::unique_ptr<ts_trimmer> ts_trimmer_;

  uint64_t packet_counter_;

public:
  boost::optional<uint16_t> transport_stream_id;
  boost::optional<program_association_table> pat;
  std::map<uint16_t, uint16_t> program_pcr;
  boost::optional<uint64_t> first_pcr;
  boost::optional<uint64_t> latest_pcr;
  boost::optional<uint64_t> baseline_pcr;
  boost::optional<time_t> baseline_time;
  std::map<
    uint16_t, // program_number
    service_descriptor> latest_service_descriptors;
};

}

#include "context_impl.hpp"

#endif
