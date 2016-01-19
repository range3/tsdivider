#ifndef _TSSP_PCR_FILTER_HPP_
#define _TSSP_PCR_FILTER_HPP_

namespace tssp
{
class context;
class transport_packet;

class pcr_filter : public filter
{
public:
  virtual ~pcr_filter() {}

  virtual void handle_packet(
      context& c,
      const transport_packet& packet);
};
}

#include "filter/pcr_filter_impl.hpp"

#endif
