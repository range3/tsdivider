#ifndef _TSSP_CONTEXT_HPP_
#define _TSSP_CONTEXT_HPP_

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "ts_reader.hpp"
#include "util.hpp"
#include "crc.hpp"

#include "view.hpp"

namespace tssp {

class filter;
class section_filter;
class pes_filter;

class context
{
public:
  context(std::unique_ptr<view> view);

  void handle_packet(const packet& p);

  void open_section_filter(
      uint16_t pid,
      std::unique_ptr<section_filter> f);
  void open_pes_filter(
      uint16_t pid,
      std::unique_ptr<pes_filter> f);

  bool is_opened(uint16_t pid) const;

  view& get_view();

  uint64_t get_packet_num() const {
    return packet_counter_;
  }

private:
  std::map<
    uint16_t,
    std::unique_ptr<filter> > pids_;

  std::unique_ptr<view> view_;

  uint64_t packet_counter_;
};

}

#include "context_impl.hpp"

#endif
