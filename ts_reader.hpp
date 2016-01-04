#ifndef _TSSP_TS_READER_HPP_
#define _TSSP_TS_READER_HPP_

#include <istream>
#include <array>
#include <cstdint>
#include <stdexcept>


namespace tssp
{
const size_t PACKET_SIZE = 188;
const size_t MAX_RESYNC_SIZE = 0xFFFF;

class packet
{
private:
  std::array<char, PACKET_SIZE> buf_;

public:
class adaptation_field_struct {
public:
  adaptation_field_struct(const packet& packet) :
    packet_(packet),
    head_(packet.data() + 4)
  {}

  size_t length() const {
    return *reinterpret_cast<const uint8_t*>(head_);
  }

  uint8_t non_discontinuity_indicator() const {
    return *reinterpret_cast<const uint8_t*>(head_+1) & 0x80;
  }

  uint8_t random_access_indicator() const {
    return *reinterpret_cast<const uint8_t*>(head_+1) & 0x40;
  }

  uint8_t elementary_stream_priority_indicator() const {
    return *reinterpret_cast<const uint8_t*>(head_+1) & 0x20;
  }

  uint8_t flag() const {
    return *reinterpret_cast<const uint8_t*>(head_+1) & 0x1F;
  }

private:
  const packet& packet_;
  const char* head_;
};

public:
  void hexdump() const{
    size_t i;
    for(i = 0; i < buf_.size(); ++i) {
      cout
        << std::setw(2)
        << std::setfill('0')
        << std::hex
        << std::uppercase
        << (int)*reinterpret_cast<const uint8_t*>(buf_.data()+i)
        << " ";
      if(i % 16 == 15)
        cout << endl;
    }
    cout << endl;
  }

  char* data() {
    return buf_.data();
  }

  const char* data() const {
    return buf_.data();
  }

  size_t size() const {
    return buf_.size();
  }

  const char* payload() const {
    return data() + 4; //FIXME: consider adaptation field
  }
  
  uint8_t sync_byte() const {
    return *reinterpret_cast<const uint8_t*>(data()); // expect to 0x47
  }

  uint8_t transport_error_indicator() const {
    return *reinterpret_cast<const uint8_t*>(data()+1) & 0x80;
  }

  uint8_t payload_uint_start_indicator() const {
    return *reinterpret_cast<const uint8_t*>(data()+1) & 0x40;
  }

  uint8_t transport_priority() const {
    return *reinterpret_cast<const uint8_t*>(data()+1) & 0x20;
  }

  uint16_t pid() const {
    return 
      ((*reinterpret_cast<const uint8_t*>(data()+1) & 0x1F) << 8) +
      *reinterpret_cast<const uint8_t*>(data()+2);
  }

  uint8_t transport_scrambling_control() const {
    return (*reinterpret_cast<const uint8_t*>(data()+3) & 0xC0) >> 6;
  }

  // 01 : payload
  // 10 : adaptation field
  // 11 : adaptation field and payload
  uint8_t adaptation_field_control() const {
    return (*reinterpret_cast<const uint8_t*>(data()+3) & 0x30) >> 4;
  }

  uint8_t continuity_index() const {
    return (*reinterpret_cast<const uint8_t*>(data()+3) & 0x0F);
  }

  const adaptation_field_struct adaptation_field() const {
    return adaptation_field_struct(*this);
  }
};

class tsreader
{
public:
  tsreader(std::istream& input) : input_(input) {}

  bool next(packet& result) {
    while(true) {
      input_.exceptions(std::istream::badbit);
      input_.read(result.data(), result.size());
      if(!input_)
        return false;

      // check packet sync byte
      if(result.sync_byte() != 0x47) {
        input_.exceptions(std::istream::failbit | std::istream::badbit);
        auto pos = input_.tellg();
        input_.seekg(
            -std::min<decltype(pos)>(pos, PACKET_SIZE),
            std::ios_base::cur);
        if(!resync())
          return false;
      }
      else {
        break;
      }
    }
    return true;
  }

private:
  bool resync() {
    char ch;
    const uint8_t* c = reinterpret_cast<uint8_t*>(&ch);
    size_t i;
    for(i = 0; i < MAX_RESYNC_SIZE; ++i) {
      input_.exceptions(std::istream::badbit);
      input_.get(ch);
      if(!input_)
        return false;
      if(*c == 0x47) {
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
