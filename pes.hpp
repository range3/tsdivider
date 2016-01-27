#ifndef _TSD_PES_HPP_
#define _TSD_PES_HPP_

#include "util.hpp"

namespace tsd
{

class pes_header
{
public:
enum class stream_type {
  program_stream_map,
  private_stream_1,
  padding_stream,
  private_stream_2,
  audio_stream,
  video_stream,
  ecm_stream,
  emm_stream,
  dsmcc_stream,
  iso_iec_13522_stream,
  itu_t_rec_h_222_1_type_a,
  itu_t_rec_h_222_1_type_b,
  itu_t_rec_h_222_1_type_c,
  itu_t_rec_h_222_1_type_d,
  itu_t_rec_h_222_1_type_e,
  ancillary_stream,
  reserved_data_stream,
  program_stream_directory,
  unknown_stream_type
};

public:
  static constexpr uint8_t packet_start_code_prefix[3] = {
    0x00, 0x00, 0x01
  };

  static bool check_packet_start(const uint8_t* p) {
    return
      get8(p) == packet_start_code_prefix[0] &&
      get8(p+1) == packet_start_code_prefix[1] &&
      get8(p+2) == packet_start_code_prefix[2];
  }

  static stream_type get_stream_type(uint8_t stream_id) {
    switch(stream_id) {
      case 0xBC:
        return stream_type::program_stream_map;
      case 0xBD:
        return stream_type::private_stream_1;
      case 0xBE:
        return stream_type::padding_stream;
      case 0xBF:
        return stream_type::private_stream_2;
      case 0xC0 ... 0xDF:
        return stream_type::audio_stream;
      case 0xE0 ... 0xEF:
        return stream_type::video_stream;
      case 0xF0:
        return stream_type::ecm_stream;
      case 0xF1:
        return stream_type::emm_stream;
      case 0xF2:
        return stream_type::dsmcc_stream;
      case 0xF3:
        return stream_type::iso_iec_13522_stream;
      case 0xF4:
        return stream_type::itu_t_rec_h_222_1_type_a;
      case 0xF5:
        return stream_type::itu_t_rec_h_222_1_type_b;
      case 0xF6:
        return stream_type::itu_t_rec_h_222_1_type_c;
      case 0xF7:
        return stream_type::itu_t_rec_h_222_1_type_d;
      case 0xF8:
        return stream_type::itu_t_rec_h_222_1_type_e;
      case 0xF9:
        return stream_type::ancillary_stream;
      case 0xFA ... 0xFE:
        return stream_type::reserved_data_stream;
      case 0xFF:
        return stream_type::program_stream_directory;
      default:
        return stream_type::unknown_stream_type;
    }
  }

  static bool have_pes_header(stream_type type) {
    return type != stream_type::program_stream_map &&
       type != stream_type::padding_stream &&
       type != stream_type::private_stream_2 &&
       type != stream_type::ecm_stream &&
       type != stream_type::emm_stream &&
       type != stream_type::program_stream_directory &&
       type != stream_type::dsmcc_stream &&
       type != stream_type::itu_t_rec_h_222_1_type_e;
  }

public:
  uint8_t stream_id;
  uint16_t pes_packet_length;
  uint16_t pes_header_block;
  uint8_t pes_header_data_length;
  uint64_t pts;
  uint64_t dts;
  uint32_t escr_base;
  uint16_t escr_extension;
  uint32_t es_rate;
  uint8_t trick_mode;
  uint8_t additional_copy_info;
  uint16_t previous_pes_packet_crc;
  std::string pes_extension_block;

  void unpack(const uint8_t** pp, const uint8_t* pend) {
    const uint8_t* p = *pp;
    const uint8_t* pheader_end;
    if(pend - p < 6)
      std::runtime_error("");
    if(!check_packet_start(p))
      std::runtime_error("");

    p += 3;
    stream_id = get8(p);
    p += 1;
    pes_packet_length = get16(p);
    p += 2;

    stream_type type = get_stream_type(stream_id);
    if(have_pes_header(type)) {
      if(pend - p < 3)
        std::runtime_error("");
      pes_header_block = get16(p);
      p += 2;
      pes_header_data_length = get8(p);
      p += 1;
      
      if(pend - p < pes_header_data_length)
        std::runtime_error("");

      pheader_end = p + pes_header_data_length;

      if(has_pts()) {
        pts = (get8(p) & 0x0E) << 29;
        p += 1;
        pts |= (get16(p) & 0xFFFE) << 14;
        p += 2;
        pts |= (get16(p) & 0xFFFE) >> 1;
        p += 2;

        if(has_dts()) {
          dts = (get8(p) & 0x0E) << 29;
          p += 1;
          dts |= (get16(p) & 0xFFFE) << 14;
          p += 2;
          dts |= (get16(p) & 0xFFFE) >> 1;
          p += 2;
        }
      }

      if(escr_flag()){
        //FIXME
        p += 6;
      }
      if(es_rate_flag()){
        es_rate = (get16(p) & 0x7FFF) << 7;
        p += 2;
        es_rate |= (get8(p) & 0xFE) >> 1;
        p += 1;
      }
      if(dsm_trick_mode_flag()) {
        trick_mode = get8(p);
        p += 1;
      }
      if(additional_copy_info_flag()) {
        additional_copy_info = get8(p) & 0x7F;
        p += 1;
      }
      if(pes_crc_flag()) {
        previous_pes_packet_crc = get16(p);
        p += 2;
      }
      if(pes_extension_flag()) {
        // include padding...
        pes_extension_block.assign(
            reinterpret_cast<const char*>(p),
            pend-p);
      }
      p = pheader_end;
    }
    *pp = p;
  }

  uint16_t pes_scrambling_control() const {
    return (pes_header_block & 0x3000) >> 12;
  }
  uint16_t pes_priority() const {
    return pes_header_block & 0x0800;
  }
  uint16_t data_alignment_indicator() const {
    return pes_header_block & 0x0400;
  }
  uint16_t copyright() const {
    return pes_header_block & 0x0200;
  }
  uint16_t original_or_copy() const {
    return pes_header_block & 0x0100;
  }
  uint16_t pts_dts_flags() const {
    return (pes_header_block & 0x00C0) >> 6;
  }
  uint16_t escr_flag() const {
    return pes_header_block & 0x0020;
  }
  uint16_t es_rate_flag() const {
    return pes_header_block & 0x0010;
  }
  uint16_t dsm_trick_mode_flag() const {
    return pes_header_block & 0x0008;
  }
  uint16_t additional_copy_info_flag() const {
    return pes_header_block & 0x0004;
  }
  uint16_t pes_crc_flag() const {
    return pes_header_block & 0x0002;
  }
  uint16_t pes_extension_flag() const {
    return pes_header_block & 0x0001;
  }

  bool has_pts() const {
    return pts_dts_flags() & 0x02;
  }
  bool has_dts() const {
    return pts_dts_flags() & 0x01;
  }
};

class pes_context
{
public:
enum class state {
  header,
  pes_header,
  pes_header_data,
  payload,
  skip
};

public:
  void init() {
    state_ = state::header;
  }

  void execute(const char* data, size_t size, size_t& off) {
    const uint8_t* pbegin = reinterpret_cast<const uint8_t*>(data); 
    const uint8_t* p = pbegin + off;
    const uint8_t* pend = pbegin + size;

    while(p < pend) {
      switch(state_) {
        case state::header:
        {
          if(pend - p < 6) {
            off = p - pbegin;
            return;
          }
          if(!pes_header::check_packet_start(p)) {
            state_ = state::skip; 
            continue;
          }
          p += 3;
          uint8_t stream_id = get8(p);
          p += 3;

          pes_header::stream_type type =
            pes_header::get_stream_type(stream_id);
          if(pes_header::have_pes_header(type)) {
            state_ = state::pes_header;
          }
          else if(type == pes_header::stream_type::padding_stream) {
            state_ = state::skip;
          }
          else {
            state_ = state::payload;
          }
        } break;
        case state::pes_header:
        {
          if(pend - p < 3) {
            off = p - pbegin;
            return;
          }
          p += 2;
          pes_header_data_length_ = get8(p);
          p += 1;
          state_ = state::pes_header_data;
        } break;
        case state::pes_header_data:
        {
          if(pend - p < pes_header_data_length_) {
            off = p - pbegin;
            return;
          }
          p += pes_header_data_length_;
          state_ = state::payload;
        } break;
        case state::payload:
        {
          p = pend;
        } break;
        case state::skip:
        {
          p = pend;
        } break;
      }
    }
    off = p - pbegin;
  }

  const state& get_state() const {
    return state_;
  }

private:
  state state_;
  uint16_t pes_header_data_length_;
};


}


#endif
