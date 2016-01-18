#ifndef _TSSP_PES_FILTER_HPP_
#define _TSSP_PES_FILTER_HPP_

#include <string>
#include "pes.hpp"

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

private:
  void handle_pes_header(
      context& c,
      const pes_header& h);
  void handle_pes_payload_stream(
      context& c,
      const pes_header& h,
      const char* data, size_t size);

public:
  std::string pes_header_buffer_;
  pes_context pes_ctx_;
  size_t offset_;
  pes_header current_header_;
};
}

#include "filter/pes_filter_impl.hpp"

#endif
