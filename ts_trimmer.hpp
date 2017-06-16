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
      std::iostream& buffer,
      int64_t trim_threshold_sec,
      bool enable_pmt_separator,
      bool enable_eit_separator,
      int overlap_front,
      int overlap_back) :
    output_(ost),
    buffer_(buffer),
    input_pos_(0),
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
    buffer_.exceptions(
        std::ios_base::failbit | 
        std::ios_base::badbit);
    // because a joint file position is maintained by std::basic_filebuf
    buffer_.seekg(input_pos_);
    std::copy(
        istreambuf_iterator<char>(buffer_),
        istreambuf_iterator<char>(),
        ostreambuf_iterator<char>(output_));
    buffer_.clear(); // clear eofbit
    input_pos_ = buffer_.tellg();
  }

  void drop_buffer(size_t remain_bytes) {
    buffer_.exceptions(
        std::ios_base::failbit | 
        std::ios_base::badbit);
    auto end_of_buffer_pos = buffer_.tellp();
    input_pos_ =
      std::max<decltype(input_pos_)>(
          end_of_buffer_pos - static_cast<std::iostream::off_type>(remain_bytes),
          input_pos_);
  }

private:
  std::ostream& output_;
  std::iostream& buffer_;
  std::iostream::pos_type input_pos_;
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
