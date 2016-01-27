#ifndef _TSD_FILTER_HPP_
#define _TSD_FILTER_HPP_

#include "transport_packet.hpp"

namespace tsd
{
class context;

class filter
{
public:
  filter() :
    last_cc_(0xf0)
  {}
  virtual ~filter() {}
  virtual bool is_section_filter() const {
    return false;
  }
  virtual void handle_packet(
      context& c,
      const transport_packet& packet) = 0;

protected:
  bool check_continuity(
      const transport_packet& packet) const {
    uint8_t expect_cc;
    if(packet.has_payload())
      expect_cc = (last_cc_+1) & 0x0f;
    else
      expect_cc = last_cc_;

    return
      packet.pid == 0x1FFF || // null packet PID
                              // FIXME: discontinuity
      expect_cc == packet.continuity_counter;
  }

protected:
  uint8_t last_cc_;
};

}

#endif
