#ifndef _TSSP_SPLITTER_CONTEXT_HPP_
#define _TSSP_SPLITTER_CONTEXT_HPP_

#include "wrap_around_time_stamp.hpp"

namespace tssp
{
class splitter_context
{
public:
  enum class state
  {
    init,
    buffering,
    writing
  };

public:
  splitter_context() :
    state_(state::init),
    trim_threshold_sec_(0) {}

  explicit splitter_context(int64_t trim_threshold_sec) :
    state_(state::init),
    trim_threshold_sec_(trim_threshold_sec) {}

  const state get_state() const {
    return state_;
  }

  void signal_pcr(uint64_t pcr) {
    switch(state_) {
    case state::init:
      {
        start_pcr_ = wrap_around_time_stamp(pcr);
        state_ = state::buffering;
      }
      break;
    case state::buffering:
      {
        wrap_around_time_stamp wpcr(pcr);
        if(wpcr < start_pcr_)
          return;

        if(pcr_to_sec(wpcr - start_pcr_) >= trim_threshold_sec_) {
          state_ = state::writing;
        }
      }
      break;
    case state::writing:
      {
      }
      break;
    }
  }

  void signal_split() {
    switch(state_) {
    case state::init:
      {
      }
      break;
    case state::buffering:
      {
        state_ = state::init;
      }
      break;
    case state::writing:
      {
        state_ = state::init;
      }
      break;
    }
  }
private:
  int64_t pcr_to_sec(int64_t pcr) const {
    return pcr / 90000;
  }

private:
  state state_;
  int64_t trim_threshold_sec_;
  wrap_around_time_stamp start_pcr_;
};

}

#endif
