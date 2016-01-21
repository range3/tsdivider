#ifndef _TSSP_PACKER_HPP_
#define _TSSP_PACKER_HPP_

#include <ostream>

namespace tssp
{
class packer
{
public:
  packer(std::ostream& ost) :
    ost_(ost)
  {}

  void pack8(uint8_t d) {
    pack(d);
  }
  void pack16(uint16_t d) {
    pack(d);
  }
  void pack32(uint32_t d) {
    pack(d);
  }
  void pack64(uint64_t d) {
    pack(d);
  }

  void pack(uint8_t d) {
    ost_.write(reinterpret_cast<const char*>(&d), 1);
  }

  void pack(uint16_t d) {
    pack(static_cast<uint8_t>(d >> 8));
    pack(static_cast<uint8_t>(d & 0xFF));
  }

  void pack(uint32_t d) {
    pack(static_cast<uint16_t>(d >> 16));
    pack(static_cast<uint16_t>(d & 0xFFFF));
  }

  void pack(uint64_t d) {
    pack(static_cast<uint32_t>(d >> 32));
    pack(static_cast<uint32_t>(d & 0xFFFFFFFF));
  }

  void pack_bytes(const char* data, size_t size) {
    ost_.write(data, size);
  }

  void pack_bytes(const uint8_t* data, size_t size) {
    pack_bytes(reinterpret_cast<const char*>(data), size);
  }

  void pack_fill(size_t n, uint8_t d) {
    for(; n > 0; --n)
      pack(d);
  }

private:
  std::ostream& ost_;
};

}


#endif
