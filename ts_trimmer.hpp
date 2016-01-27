#ifndef _TSD_TS_TRIMMER_HPP_
#define _TSD_TS_TRIMMER_HPP_

#include <ostream>
#include <sstream>
#include "splitter_context.hpp"
#include "transport_packet.hpp"

namespace tsd
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
      int64_t trim_threshold_sec,
      bool enable_pmt_separator,
      bool enable_eit_separator,
      int overlap_front,
      int overlap_back) :
    output_(ost),
    buffer_(),
    context_(trim_threshold_sec),
    state_(state::buffering),
    enable_pmt_separator_(enable_pmt_separator),
    enable_eit_separator_(enable_eit_separator),
    overlap_front_(overlap_front),
    overlap_back_(overlap_back),
    written_overlap_back_(0)
  {}

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
        if(written_overlap_back_ < overlap_back_){
          packet.pack(output_);
          written_overlap_back_ += 1;
        }
        else {
          packet.pack(buffer_);
        }
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
          written_overlap_back_ = 0;
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
        drop_buffer(0);
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
        drop_buffer(0);
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
        drop_buffer(overlap_front_*transport_packet::size);
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
    drop_buffer(0);
  }

  void drop_buffer(int remain_bytes) {
    std::string new_buffer;
    if(remain_bytes > 0) {
      buffer_.exceptions(std::ios_base::badbit);
      buffer_.seekg(0, std::ios_base::end);
      auto length = buffer_.tellg();
      auto overlap_front =
        std::min<decltype(length)>(
            length,
            remain_bytes);
      buffer_.seekg(length - overlap_front, std::ios_base::beg);
      new_buffer.resize(overlap_front);
      buffer_.read(&*new_buffer.begin(), new_buffer.size());
    }
    buffer_.str(new_buffer);
    buffer_.seekp(0, std::ios_base::end);
    buffer_.clear();
  }

private:
  std::ostream& output_;
  std::stringstream buffer_;
  splitter_context context_;
  state state_;
  bool enable_pmt_separator_;
  bool enable_eit_separator_;
  int overlap_front_;
  int overlap_back_;
  int written_overlap_back_;
};

}


#endif
