#ifndef _TSSP_PCR_FILTER_IMPL_HPP_
#define _TSSP_PCR_FILTER_IMPL_HPP_

#include "context.hpp"
#include "transport_packet.hpp"


namespace tssp
{

void pcr_filter::handle_packet(
    context& c,
    const transport_packet& packet) {
//  cout << "pcr : " << packet.afield.pcr_base / 90000<< endl;
}

}




#endif
