#ifndef _TSSP_VIEW_HPP_
#define _TSSP_VIEW_HPP_

#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include "picojson.h"
#include "aribstr.h"
#include "util.hpp"
#include "section_header.hpp"
#include "pat.hpp"
#include "pmt.hpp"
#include "sdt.hpp"
#include "tot.hpp"
#include "eit.hpp"
#include "descriptor.hpp"

namespace tssp
{

class view
{
public:
  view() :
    print_section_header_(false),
    print_pat_(false),
    print_pmt_(false),
    print_sdt_(false),
    print_tot_(false),
    print_eit_(false),
    print_if_changed_(false),
    print_packet_num_(false)
  {}

  virtual ~view() {}

  void set_print_section_header(bool p) {
    print_section_header_ = p;
  }
  void set_print_pat(bool p) {
    print_pat_ = p;
  }
  void set_print_pmt(bool p) {
    print_pmt_ = p;
  }
  void set_print_sdt(bool p) {
    print_sdt_ = p;
  }
  void set_print_tot(bool p) {
    print_tot_ = p;
  }
  void set_print_eit(bool p) {
    print_eit_ = p;
  }
  void set_print_if_changed(bool p) {
    print_if_changed_ = p;
  }
  void set_print_packet_num(bool p) {
    print_packet_num_ = p;
  }

  void print(
      uint64_t packet_num,
      const section_header& header,
      const program_association_table& pat,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_pat_) {
      print_packet_num(packet_num);
      print_section_header(header);
      on_print(pat);
    }
  }
  void print(
      uint64_t packet_num,
      const section_header& header,
      const program_map_table& pmt,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_pmt_) {
      print_packet_num(packet_num);
      print_section_header(header);
      on_print(pmt);
    }
  }
  void print(
      uint64_t packet_num,
      const section_header& header,
      const service_description_table& sdt,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_sdt_) {
      print_packet_num(packet_num);
      print_section_header(header);
      on_print(sdt);
    }
  }
  void print(
      uint64_t packet_num,
      const section_header& header,
      const time_offset_table& tot,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_tot_) {
      print_packet_num(packet_num);
      print_section_header(header);
      on_print(tot);
    }
  }
  void print(
      uint64_t packet_num,
      const section_header& header,
      const event_information_table& eit,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_eit_) {
      print_packet_num(packet_num);
      print_section_header(header);
      on_print(eit);
    }
  }

private:
  void print_packet_num(uint64_t n) const {
    if(print_packet_num_)
      on_print_packet_num(n);
  }

  void print_section_header(const section_header& h) const {
    if(print_section_header_)
      on_print_section_header(h);
  }

protected:
  virtual void on_print_packet_num(uint64_t n) const {
    cout << std::dec << n << "\t";
  }

  virtual void on_print_section_header(const section_header& header) const {}
  virtual void on_print(const program_association_table& pat) const {}
  virtual void on_print(const program_map_table& pmt) const {}
  virtual void on_print(const service_description_table& sdt) const {}
  virtual void on_print(const time_offset_table& tot) const {}
  virtual void on_print(const event_information_table& eit) const {}

protected:
  bool print_section_header_;
  bool print_pat_;
  bool print_pmt_;
  bool print_sdt_;
  bool print_tot_;
  bool print_eit_;
  bool print_if_changed_;
  bool print_packet_num_;
};


class json_view : public view
{
public:
  json_view() :
    prettify_(false) {}
  explicit json_view(bool prettify) :
    prettify_(prettify) {}
  virtual ~json_view() {}

protected:
  virtual void on_print_section_header(const section_header& h) const {
    cout << serialize_section_header(h).serialize(prettify_);
  }

  virtual void on_print(const program_association_table& pat) const {
    picojson::object o;
    picojson::array association;

    for(auto& i : pat.association) {
      picojson::object program_to_pid_obj;
      program_to_pid_obj.emplace(
          "program_number", picojson::value(d(i.program_number)));
      program_to_pid_obj.emplace(
          "pmt_pid", picojson::value(d(i.pmt_pid)));
      association.emplace_back(
          picojson::value(program_to_pid_obj));
    }

    o.emplace(
        "association",
        picojson::value(association));

    cout << picojson::value(o).serialize(prettify_) << endl;
  }

  virtual void on_print(const program_map_table& pmt) const {
    picojson::object o;
    o.emplace(
        "pcr_pid",
        picojson::value(d(pmt.pcr_pid)));

    picojson::array program_info;
    for(auto& pi : pmt.program_info) {
      picojson::object pio;
      pio.emplace("tag", picojson::value(d(pi.tag)));
      program_info.emplace_back(picojson::value(pio));
    }
    o.emplace("program_info", picojson::value(program_info));

    picojson::array program_elements;
    for(auto& pe : pmt.program_elements) {
      picojson::object peo;
      peo.emplace("stream_type", picojson::value(d(pe.stream_type)));
      peo.emplace("elementary_pid", picojson::value(d(pe.elementary_pid)));
      picojson::array es_info;
      for(auto& desc : pe.es_info) {
        picojson::object es_info_o;
        es_info_o.emplace("tag", picojson::value(d(desc.tag)));
        if(desc.tag == stream_identifier_descriptor::TAG) {
          auto sid = desc.as<stream_identifier_descriptor>();
          es_info_o.emplace("component_tag", picojson::value(d(sid->component_tag)));
        }
        es_info.emplace_back(picojson::value(es_info_o));
      }
      peo.emplace("es_info", picojson::value(es_info));
      program_elements.emplace_back(picojson::value(peo));
    }
    o.emplace("program_elements", picojson::value(program_elements));

    cout << picojson::value(o).serialize(prettify_) << endl;
  }

  virtual void on_print(const service_description_table& sdt) const {
    char tmpbuf[4096];
    picojson::object o;
    o.emplace(
        "original_network_id",
        picojson::value(d(sdt.original_network_id)));

    picojson::array services;
    for(auto& s : sdt.services) {
      picojson::object sobj;
      sobj.emplace("service_id", picojson::value(d(s.service_id)));
      picojson::array descriptors;
      for(auto& desc : s.descriptors) {
        if(desc.tag == service_descriptor::TAG) {
          auto sd = desc.as<service_descriptor>();
          picojson::object dobj;
          dobj.emplace("service_type", picojson::value(d(sd->service_type)));
          AribToString(
              tmpbuf,
              sd->service_provider_name.data(),
              sd->service_provider_name.size());
          dobj.emplace("provider_name", picojson::value(tmpbuf));
          AribToString(
              tmpbuf,
              sd->service_name.data(),
              sd->service_name.size());
          dobj.emplace("service_name", picojson::value(tmpbuf));
          descriptors.emplace_back(picojson::value(dobj));
        }
      }
      sobj.emplace("descriptors", picojson::value(descriptors));
      services.emplace_back(picojson::value(sobj));
    }
    o.emplace("services", picojson::value(services));
    cout << picojson::value(o).serialize(prettify_) << endl;
  }

  virtual void on_print(const time_offset_table& tot) const {
    picojson::object o;
    time_t t = tot.time.to_time_t();
    char tmpbuf[100];
    std::strftime(tmpbuf, sizeof(tmpbuf), "%c %Z", std::localtime(&t));
    o.emplace("jst_time", picojson::value(tmpbuf));
    cout << picojson::value(o).serialize(prettify_) << endl;
  }

  virtual void on_print(const event_information_table& eit) const {
    picojson::object rooto;
    rooto.emplace(
        "transport_stream_id", picojson::value(d(eit.transport_stream_id)));
    rooto.emplace(
        "original_network_id", picojson::value(d(eit.original_network_id)));
    rooto.emplace(
        "segment_last_section_number", picojson::value(d(eit.segment_last_section_number)));
    rooto.emplace(
        "last_table_id", picojson::value(d(eit.last_table_id)));

    std::ostringstream oss;
    char tmpbuf[4096];
    time_t t;
    picojson::array events;
    for(auto& e : eit.events) {
      picojson::object eo;
      eo.emplace("event_id", picojson::value(d(e.event_id)));
      t = e.start_time.to_time_t();
      std::strftime(tmpbuf, sizeof(tmpbuf), "%c %Z", std::localtime(&t));
      eo.emplace("start_time", picojson::value(tmpbuf));
      oss.str("");
      oss
        << static_cast<int>(e.duration.hour) << ':'
        << static_cast<int>(e.duration.min) << ':'
        << static_cast<int>(e.duration.sec);
      eo.emplace(
          "duration", picojson::value(oss.str()));
      eo.emplace(
          "running_status", picojson::value(d(e.running_status)));
      eo.emplace(
          "free_ca_mode", picojson::value(static_cast<bool>(e.free_ca_mode)));

      picojson::array descriptors;
      for(auto& desc : e.descriptors) {
        picojson::object dobj;
        dobj.emplace("tag", picojson::value(d(desc.tag)));
        if(desc.tag == short_event_descriptor::TAG) {
          auto sed = desc.as<short_event_descriptor>();
          dobj.emplace(
              "iso_639_language_code",
              picojson::value(sed->iso_639_language_code));
          AribToString(
              tmpbuf,
              sed->event_name.data(),
              sed->event_name.size());
          dobj.emplace(
              "event_name",
              picojson::value(tmpbuf));
          AribToString(
              tmpbuf,
              sed->text.data(),
              sed->text.size());
          dobj.emplace(
              "text",
              picojson::value(tmpbuf));
        }
        descriptors.emplace_back(picojson::value(dobj));
      }
      eo.emplace("descriptors", picojson::value(descriptors));
      events.emplace_back(picojson::value(eo));
    }
    rooto.emplace("events", picojson::value(events));

    cout << picojson::value(rooto).serialize(prettify_) << endl;
  }

private:
  picojson::value serialize_section_header(
      const section_header& sh) const {
    picojson::object o;
    o.emplace(
        "table_id", picojson::value(d(sh.table_id)));
    if(sh.section_syntax_indicator) {
      o.emplace(
          "table_id_extension", picojson::value(d(sh.table_id_extension)));
      o.emplace(
          "version", picojson::value(d(sh.version)));
      o.emplace(
          "current_next_indicator", picojson::value(d(sh.current_next_indicator)));
      o.emplace(
          "section_number", picojson::value(d(sh.section_number)));
      o.emplace(
          "last_section_number", picojson::value(d(sh.section_number)));
    }
    return picojson::value(o);
  }

  template<typename D>
  double d(const D& d) const {
    return static_cast<double>(d);
  }

private:
  bool prettify_;

};


class debug_view : public view
{
public:
  virtual ~debug_view() {}

protected:
  virtual void on_print_section_header(const section_header& h) const {
    cout << "-----section header-----" << endl;
    dump_section_header(h);
  }

  virtual void on_print(const program_association_table& pat) const {
    cout << "----- pat -----" << endl;
    for(auto& i : pat.association) {
      cout << "[program : " << i.program_number;
      cout << ", pid : " << i.pmt_pid << "]" << endl;
    }
  }

  virtual void on_print(const program_map_table& pmt) const {
    cout << "----- pmt -----" << endl;
    cout << "pcr_pid : " << (int)pmt.pcr_pid << endl;
    cout << "program_info(descriptor)" << endl;
    {
      for(auto& i : pmt.program_info) {
        cout << "\t" << "tag : " << (int)i.tag << endl;
        cout << "\t" << "length : " << (int)i.length << endl;
      }
    }
    cout << "program_elements" << endl;
    {
      for(auto& i : pmt.program_elements) {
        cout << "\t" << "stream_type : " << (int)i.stream_type << endl;
        cout << "\t" << "elementary_pid : " << (int)i.elementary_pid << endl;
        cout << "\t" << "es_info(descriptor)" << endl;
        for(auto& j : i.es_info) {
          cout << "\t\t" << "tag : " << (int)j.tag << endl;
          cout << "\t\t" << "length : " << (int)j.length << endl;
        }
      }
    }
  }

  virtual void on_print(const service_description_table& sdt) const {
    cout << "----SDT sectoin----" << endl;
    char tmpbuf[4096];
    cout << "services : " << endl;
    for(auto& s : sdt.services) {
      cout << "\t" << "service_id : " << (int)s.service_id << endl;
      for(auto& d : s.descriptors) {
        cout << "\t\t" << "tag : " << (int)d.tag << endl;
        cout << "\t\t" << "length : " << (int)d.length << endl;
        if(d.tag == service_descriptor::TAG) {
          auto sd = d.as<service_descriptor>();
          cout << "\t\t" << "service_type: " << (int)sd->service_type << endl;
          AribToString(
              tmpbuf,
              sd->service_provider_name.data(),
              sd->service_provider_name.size());
          cout << "\t\t" << "service_provider_name: " << tmpbuf << endl;
          AribToString(
              tmpbuf,
              sd->service_name.data(),
              sd->service_name.size());
          cout << "\t\t" << "service_name: " << tmpbuf << endl;
        }
      }
    }
  }

  virtual void on_print(const time_offset_table& tot) const {
    cout << "----- tot section -----" << endl;
    cout << dec;
    time_t t = tot.time.to_time_t();
    char tmpbuf[100];
    std::strftime(tmpbuf, sizeof(tmpbuf), "%c %Z", std::localtime(&t));
    cout << "time : " << tmpbuf << endl;
  }

  virtual void on_print(const event_information_table& eit) const {
    cout << "----- EIT section -----" << endl;
  }

private:
  void dump_section_header(const section_header& sh) const {
    cout << "table id : " << (int)sh.table_id << endl;
    if(sh.section_syntax_indicator) {
      cout << "table_id_extension : " << (int)sh.table_id_extension << endl;
      cout << "version : " << (int)sh.version << endl;
      cout << "current_next_indicator : " << (int)sh.current_next_indicator << endl;
      cout << "section number : " << (int)sh.section_number << endl;
      cout << "last section number : " << (int)sh.last_section_number << endl;
    }
  }
};

}

#endif
