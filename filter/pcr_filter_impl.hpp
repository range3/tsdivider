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
  if(c.pat){
    for(auto& i : c.pat->association) {
      if(i.program_number != 0) {
        auto i_pcr = c.program_pcr.find(i.program_number);
        if(i_pcr != c.program_pcr.end()) {
          if(i_pcr->second == packet.pid) {
            c.signal_pcr(packet.afield.pcr_base);
          }
        }
        break;
      }
    }
  }
}

}




#endif
