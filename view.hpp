#ifndef _TSSP_VIEW_HPP_
#define _TSSP_VIEW_HPP_

#include <iostream>
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

namespace tssp
{

class view
{
public:
  view() :
    print_pat_(false),
    print_pmt_(false),
    print_sdt_(false),
    print_tot_(false),
    print_if_changed_(false),
    print_packet_num_(false)
  {}

  virtual ~view() {}

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
  void set_print_if_changed(bool p) {
    print_if_changed_ = p;
  }
  void set_print_packet_num(bool p) {
    print_packet_num_ = p;
  }

  void print(
      uint64_t packet_num,
      const program_association_table& pat,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_pat_) {
      print_packet_num(packet_num);
      on_print(pat);
    }
  }
  void print(
      uint64_t packet_num,
      const program_map_table& pmt,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_pmt_) {
      print_packet_num(packet_num);
      on_print(pmt);
    }
  }
  void print(
      uint64_t packet_num,
      const service_description_table& sdt,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_sdt_) {
      print_packet_num(packet_num);
      on_print(sdt);
    }
  }
  void print(
      uint64_t packet_num,
      const time_offset_table& tot,
      bool changed = true) const {
    if(print_if_changed_ && !changed)
      return;
    if(print_tot_) {
      print_packet_num(packet_num);
      on_print(tot);
    }
  }

private:
  void print_packet_num(uint64_t n) const {
    if(print_packet_num_)
      on_print_packet_num(n);
  }

protected:
  virtual void on_print_packet_num(uint64_t n) const {
    cout << std::dec << n << "\t";
  }

  virtual void on_print(const program_association_table& pat) const {}
  virtual void on_print(const program_map_table& pmt) const {}
  virtual void on_print(const service_description_table& sdt) const {}
  virtual void on_print(const time_offset_table& tot) const {}

protected:
  bool print_pat_;
  bool print_pmt_;
  bool print_sdt_;
  bool print_tot_;
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
  virtual void on_print(const program_association_table& pat) const {
    picojson::value root = serialize_section_header(pat.header);
    picojson::object& o = root.get<picojson::object>();
    picojson::array program_num_to_pid;

    for(auto& kv : pat.program_num_to_pid) {
      picojson::object program_to_pid_obj;
      program_to_pid_obj.emplace("program", picojson::value(d(kv.first)));
      program_to_pid_obj.emplace("pid", picojson::value(d(kv.second)));
      program_num_to_pid.emplace_back(picojson::value(program_to_pid_obj));
    }

    o.emplace(
        "program_to_pid",
        picojson::value(program_num_to_pid));

    cout << root.serialize(prettify_) << endl;
  }

  virtual void on_print(const program_map_table& pmt) const {
    picojson::value root = serialize_section_header(pmt.header);
    picojson::object& o = root.get<picojson::object>();
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
      peo.emplace("type", picojson::value(d(pe.stream_type)));
      peo.emplace("pid", picojson::value(d(pe.elementary_pid)));
      picojson::array es_info;
      for(auto& desc : pe.es_info) {
        picojson::object es_info_o;
        es_info_o.emplace("tag", picojson::value(d(desc.tag)));
        es_info.emplace_back(picojson::value(es_info_o));
      }
      peo.emplace("es_info", picojson::value(es_info));
      program_elements.emplace_back(picojson::value(peo));
    }
    o.emplace("program_elements", picojson::value(program_elements));

    cout << root.serialize(prettify_) << endl;
  }

  virtual void on_print(const service_description_table& sdt) const {
    char tmpbuf[4096];
    picojson::value root = serialize_section_header(sdt.header);
    picojson::object& o = root.get<picojson::object>();
    o.emplace(
        "orig_network_id",
        picojson::value(d(sdt.original_network_id)));

    picojson::array services;
    for(auto& s : sdt.services) {
      picojson::object sobj;
      sobj.emplace("sid", picojson::value(d(s.service_id)));
      picojson::array descriptors;
      for(auto& desc : s.descriptors) {
        if(desc.tag == service_descriptor::TAG) {
          auto sd = desc.as<service_descriptor>();
          picojson::object dobj;
          dobj.emplace("type", picojson::value(d(sd->service_type)));
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
    cout << root.serialize(prettify_) << endl;
  }

  virtual void on_print(const time_offset_table& tot) const {
    picojson::object o;
    o.emplace("tid", picojson::value(d(tot.table_id)));
    time_t t = std::chrono::system_clock::to_time_t(tot.time());
    char tmpbuf[100];
    std::strftime(tmpbuf, sizeof(tmpbuf), "%c %Z", std::localtime(&t));
    o.emplace("time", picojson::value(tmpbuf));
    cout << picojson::value(o).serialize(prettify_) << endl;
  }

private:
  picojson::value serialize_section_header(
      const section_header& sh) const {
    picojson::object o;
    o.emplace(
        "tid", picojson::value(d(sh.table_id)));
    o.emplace(
        "tsid", picojson::value(d(sh.transport_stream_id)));
    o.emplace(
        "version", picojson::value(d(sh.version)));
    o.emplace(
        "sec_num", picojson::value(d(sh.section_number)));
    o.emplace(
        "last_sec_num", picojson::value(d(sh.section_number)));
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
  virtual void on_print(const program_association_table& pat) const {
    cout << "----- pat -----" << endl;
    dump_section_header(pat.header);
    for(auto& kv : pat.program_num_to_pid) {
      cout << "[program : " << kv.first;
      cout << ", pid : " << kv.second << "]" << endl;
    }
  }

  virtual void on_print(const program_map_table& pmt) const {
    cout << "----- pmt -----" << endl;
    dump_section_header(pmt.header);
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
    cout << "header : " << endl;
    dump_section_header(sdt.header);
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
    cout << "table id : " << (int)tot.table_id << endl;
    cout << dec;
    time_t t = std::chrono::system_clock::to_time_t(tot.time());
    char tmpbuf[100];
    std::strftime(tmpbuf, sizeof(tmpbuf), "%c %Z", std::localtime(&t));
    cout << "time : " << tmpbuf << endl;
  }

private:
  void dump_section_header(const section_header& sh) const {
    cout << "table id : " << (int)sh.table_id << endl;
    cout << "tranport stream id : " << (int)sh.transport_stream_id << endl;
    cout << "version : " << (int)sh.version << endl;
    cout << "section number : " << (int)sh.section_number << endl;
    cout << "last section number : " << (int)sh.last_section_number << endl;
  }
};

}

#endif
