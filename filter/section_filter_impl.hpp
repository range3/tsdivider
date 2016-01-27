#ifndef _TSD_SECTION_FILTER_IMPL_HPP_
#define _TSD_SECTION_FILTER_IMPL_HPP_

#include "context.hpp"
#include "transport_packet.hpp"

namespace tsd
{

void section_filter::handle_packet(
    context& c,
    const transport_packet& packet) {
  auto p = packet.payload;
  auto pend = p + packet.payload_size();
  auto cc_ok = check_continuity(packet);

  last_cc_ = packet.continuity_counter;

  if(packet.payload_unit_start_indicator) {
    // pointer field present
    auto pf = packet.pointer_field();
    p += 1;
    if(pf > pend - p)
      return;

    if(pf > 0 && cc_ok) {
      // write remaining section bytes
      write_data(c, p, pf, false);
    }

    p += pf;
    if(p < pend) {
      write_data(c, p, pend - p, true);
    }
  }
  else {
    if(cc_ok) {
      write_data(c, p, pend - p, false);
    }
  }
}

void section_filter::write_data(
    context& c,
    const char* data,
    size_t size,
    bool is_start) {
  if(is_start) {
    section_buffer_.assign(data, size);
  }
  else {
    section_buffer_.append(data, size);
  }

  if(section_buffer_.size() >= 3) {
    uint16_t section_length =
      (get16(section_buffer_.data()+1) & 0x0FFF) + 3;
    if(section_length > 4096) {
      return; //too long
    }
    if(section_buffer_.size() >= section_length) {
      bool crc_valid = true;
      if(do_crc_check_) {
        if(section_length < 4)
          return;
        uint32_t crc32 = get32(section_buffer_.data()+section_length-4);
        crc32_ts crc_calc;
        crc_calc.process_bytes(
            section_buffer_.data(),
            section_length-4);
        crc_valid = (crc32 == crc_calc());
      }

      if(crc_valid) {
        handle_section(c, section_buffer_.data(), section_length);
      }
    }
  }
}

inline
void section_filter::handle_section(
    context& c,
    const char* section_buffer,
    size_t section_length) {
  //DEBUG
  //cerr << "-----section dump-----" << endl;
  //tsd::hexdump(section_buffer, section_length, std::cerr);
  do_handle_section(c, section_buffer, section_length);
}

}


#endif
