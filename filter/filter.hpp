#ifndef _TSSP_FILTER_HPP_
#define _TSSP_FILTER_HPP_

namespace tssp
{
class context;

class filter
{
public:
  filter() :
    last_ci_(0xf0)
  {}
  virtual ~filter() {}
  virtual bool is_section_filter() const {
    return false;
  }
  virtual void write_data(
      context& c,
      const char* data,
      size_t size,
      bool is_start) = 0;


  uint8_t last_ci() const {
    return last_ci_;
  }

  void set_last_ci(uint8_t ci) {
    last_ci_ = ci;
  }

protected:
  uint8_t last_ci_;
};

}

#endif
