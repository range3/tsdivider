#ifndef _TSSP_TS_TRIMMER_HPP_
#define _TSSP_TS_TRIMMER_HPP_

#include <ostream>
#include <sstream>
#include "splitter_context.hpp"
#include "transport_packet.hpp"

namespace tssp
{

class ts_trimmer
{
  enum class state
  {
    buffering,
    writing,
    buffering2,
    eof
  };
public:
  ts_trimmer(
      std::ostream& ost,
      int64_t trim_threshold_sec) :
    output_(ost),
    buffer_(),
    context_(trim_threshold_sec),
    state_(state::buffering),
    enable_pmt_separator_(true),
    enable_eit_separator_(true) {}

  void enable_pmt_separator(bool b) {
    enable_pmt_separator_ = b;
  }
  void enable_eit_separator(bool b) {
    enable_eit_separator_ = b;
  }

  void add_packet(const transport_packet& packet) {
    switch(state_) {
    case state::buffering:
      {
        packet.pack(buffer_);
      }
      break;
    case state::writing:
      {
        packet.pack(output_);
      }
      break;
    case state::buffering2:
      {
        packet.pack(buffer_);
      }
      break;
    case state::eof:
      {
      }
      break;
    }
  }

  void signal_pmt() {
    if(enable_pmt_separator_)
      signal_split();
  }
  
  void signal_eit() {
    if(enable_eit_separator_)
      signal_split();
  }

  void signal_pcr(uint64_t pcr) {
    switch(state_) {
    case state::buffering:
      {
        context_.signal_pcr(pcr);
        if(context_.get_state() ==
            splitter_context::state::writing) {
          flush_buffer();
          state_ = state::writing;
        }
      }
      break;
    case state::writing:
      {
        context_.signal_pcr(pcr);
      }
      break;
    case state::buffering2:
      {
        context_.signal_pcr(pcr);
        if(context_.get_state() ==
            splitter_context::state::writing){
          flush_buffer();
          state_ = state::writing;
        }
      }
      break;
    case state::eof:
      {
      }
      break;
    }
  }

  void signal_finish_stream() {
    switch(state_) {
    case state::buffering:
      {
        drop_buffer();
        state_ = state::eof;
      }
      break;
    case state::writing:
      {
        state_ = state::eof;
      }
      break;
    case state::buffering2:
      {
        drop_buffer();
        state_ = state::eof;
      }
      break;
    case state::eof:
      {
      }
      break;
    }
  }

private:
  void signal_split() {
    switch(state_) {
    case state::buffering:
      {
        context_.signal_split();
        drop_buffer();
      }
      break;
    case state::writing:
      {
        context_.signal_split();
        state_ = state::buffering2;
      }
      break;
    case state::buffering2:
      {
      }
      break;
    case state::eof:
      {
      }
      break;
    }
  }
  
  void flush_buffer() {
    std::copy(
        istreambuf_iterator<char>(buffer_),
        istreambuf_iterator<char>(),
        ostreambuf_iterator<char>(output_));
    drop_buffer();
  }

  void drop_buffer() {
    buffer_.str(std::string());
    buffer_.clear();
  }

private:
  std::ostream& output_;
  std::stringstream buffer_;
  splitter_context context_;
  state state_;
  bool enable_pmt_separator_;
  bool enable_eit_separator_;
};

}


#endif
