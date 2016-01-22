#ifndef _TSSP_WRAP_AROUND_TIME_STAMP_HPP
#define _TSSP_WRAP_AROUND_TIME_STAMP_HPP


namespace tssp
{
class wrap_around_time_stamp
{
public:
  static constexpr int64_t max = 0x1FFFFFFFF;

public:
  wrap_around_time_stamp() :
    time_(0) {}
  explicit wrap_around_time_stamp(int64_t t) :
    time_(t) {}
  explicit wrap_around_time_stamp(uint64_t t) :
    time_(t) {}

  int64_t get() const {
    return time_;
  }

  const int64_t operator-(
      const wrap_around_time_stamp& o) const {
    if(std::abs(time_ - o.time_) <= max/2) {
      return time_ - o.time_;
    }
    else {
      if(time_ < o.time_)
        return (time_+max+1) - o.time_;
      else
        return time_ - (o.time_+max+1);
    }
  }

  bool operator<(const wrap_around_time_stamp& o) const {
    if(std::abs(time_ - o.time_) <= max/2)
      return time_ < o.time_;
    else
      return o.time_ < time_;
  }


private:
  int64_t time_;
};

}


#endif
