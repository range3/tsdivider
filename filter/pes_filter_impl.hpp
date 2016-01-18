#ifndef _TSSP_PES_FILTER_IMPL_HPP_
#define _TSSP_PES_FILTER_IMPL_HPP_

#include "context.hpp"
#include "util.hpp"
#include "pes.hpp"

namespace tssp
{

void pes_filter::write_data(
    context& c,
    const char* data,
    size_t size,
    bool is_start) {
  if(is_start) {
    pes_ctx_.init();
    offset_ = 0;
    pes_header_buffer_.clear();
  }

  if(pes_ctx_.get_state() == pes_context::state::skip) {
    return;
  }
  else if(pes_ctx_.get_state() == pes_context::state::payload) {
    handle_pes_payload_stream(c, current_header_, data, size);
  }
  else {
    pes_header_buffer_.append(data, size);
    pes_ctx_.execute(
        pes_header_buffer_.data(),
        pes_header_buffer_.size(),
        offset_);

    if(pes_ctx_.get_state() == pes_context::state::payload) {
      const uint8_t* p = reinterpret_cast<const uint8_t*>(
          pes_header_buffer_.data());
      const uint8_t* pend = p+pes_header_buffer_.size();
      current_header_.unpack(&p, pend);
      handle_pes_header(c, current_header_);
      handle_pes_payload_stream(
          c,
          current_header_, 
          reinterpret_cast<const char*>(p),
          pend - p);
    }
  }
}

void pes_filter::handle_pes_header(
    context& c,
    const pes_header& h) {
  //pes_header::stream_type type =
  //  pes_header::get_stream_type(h.stream_id);
  //if(pes_header::have_pes_header(type)) {
  //  //cout << "---- pes packet begin ----" << endl;
  //  //cout << "stream_type : " << (int)h.stream_id << endl;
  //  //cout << "pes_packet_length : " << (int)h.pes_packet_length << endl;
  //  //cout << "pes_header_data_length : " << (int)h.pes_header_data_length << endl;
  //  if(h.has_pts()){
  //    cout << "pts : " << h.pts / 90000 << endl;
  //    //cout << "pts : " << pts_sec << endl;
  //  }
  //  //if(h.has_dts())
  //  //  cout << "dts : " << h.dts << endl;
  //}
}

void pes_filter::handle_pes_payload_stream(
    context& c,
    const pes_header& h,
    const char* data, size_t size) {

}

}



#endif
