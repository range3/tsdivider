#ifndef _TSSP_PES_FILTER_HPP_
#define _TSSP_PES_FILTER_HPP_

#include <string>

namespace tssp
{
class context;

class pes_filter : public filter
{
public:
  virtual ~pes_filter() {}

  virtual void write_data(
      context& c,
      const char* data,
      size_t size,
      bool is_start);
public:
  std::string pes_buffer_;
};
}

#include "filter/pes_filter_impl.hpp"

#endif
