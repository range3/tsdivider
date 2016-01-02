#ifndef _TSSP_CRC_HPP_
#define _TSSP_CRC_HPP_

#include <boost/crc.hpp>

namespace tssp
{
typedef boost::crc_optimal<32, 0x04C11DB7, 0xFFFFFFFF, 0, false, false> crc32_ts;
}


#endif
