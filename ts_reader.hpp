#ifndef _TSSP_TS_READER_HPP_
#define _TSSP_TS_READER_HPP_

#include <iostream>
#include <istream>
#include <array>
#include <cstdint>
#include <stdexcept>

#include "util.hpp"
#include "transport_packet.hpp"


namespace tssp
{
class tsreader
{
  std::array<char, transport_packet::size> buffer_;
public:
  static const size_t max_resync_size = 0xFFFF;

public:
  tsreader(std::istream& input) : input_(input) {}

  bool next(transport_packet& result) {
    while(true) {
      input_.exceptions(std::istream::badbit);
      input_.read(buffer_.data(), buffer_.size());
      if(!input_)
        return false;

      // check packet sync byte
      if(buffer_[0] != transport_packet::sync_byte) {
        input_.exceptions(std::istream::failbit | std::istream::badbit);
        auto pos = input_.tellg();
        input_.seekg(
            -std::min<decltype(pos)>(pos, transport_packet::size),
            std::ios_base::cur);
        if(!resync())
          return false;
      }
      else {
        break;
      }
    }
    result.unpack(buffer_.data(), buffer_.size());
    return true;
  }

private:
  bool resync() {
    char ch;
    const uint8_t* c = reinterpret_cast<uint8_t*>(&ch);
    size_t i;
    for(i = 0; i < max_resync_size; ++i) {
      input_.exceptions(std::istream::badbit);
      input_.get(ch);
      if(!input_)
        return false;
      if(*c == transport_packet::sync_byte) {
        input_.exceptions(std::istream::failbit | std::istream::badbit);
        input_.seekg(-1, std::ios_base::cur);
        return true;
      }
    }
    throw std::runtime_error("invalid data. cannot find 0x47 sync byte");
  }

private:
  std::istream& input_;
};

} // namespace tssp

#endif
