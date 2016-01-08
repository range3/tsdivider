#ifndef _TSSP_PRINTER_HPP_
#define _TSSP_PRINTER_HPP_


#include <ostream>
#include <string>
#include "picojson.h"

#include "util.hpp"
#include "section_header.hpp"
#include "sdt.hpp"

namespace tssp
{


template<typename T>
class printer
{
  std::ostream& ost_;
public:
  printer(std::ostream& ost) :
    ost_(ost)
  {}

  void print_json(const T& data, bool prettify);
  void print_debug(const T& data);

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

  void dump_section_header(const section_header& sh) const {
    ost_ << "table id : " << (int)sh.table_id << endl;
    ost_ << "tranport stream id : " << (int)sh.transport_stream_id << endl;
    ost_ << "version : " << (int)sh.version << endl;
    ost_ << "section number : " << (int)sh.section_number << endl;
    ost_ << "last section number : " << (int)sh.last_section_number << endl;
  }
};


template<typename T>
inline
void printer<T>::print_json(
    const T& data,
    bool prettify) {}

template<typename T>
inline
void printer<T>::print_debug(const T& data){}

template<>
inline
void printer<program_association_table>::print_json(
    const program_association_table& pat,
    bool prettify) {
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

  ost_ << root.serialize(prettify) << endl;
}

template<>
inline
void printer<service_description_table>::print_json(
    const service_description_table& data,
    bool prettify) {
  char tmpbuf[4096];
  picojson::value root = serialize_section_header(data.header);
  picojson::object& o = root.get<picojson::object>();
  o.emplace(
      "orig_network_id",
      picojson::value(d(data.original_network_id)));

  picojson::array services;
  for(auto& s : data.services) {
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
  ost_ << root.serialize(prettify) << endl;
}

template<>
inline
void printer<service_description_table>::print_debug(
    const service_description_table& sdt) {
  char tmpbuf[4096];
  ost_ << "header : " << endl;
  dump_section_header(sdt.header);
  ost_ << "services : " << endl;
  for(auto& s : sdt.services) {
    ost_ << "\t" << "service_id : " << (int)s.service_id << endl;
    for(auto& d : s.descriptors) {
      ost_ << "\t\t" << "tag : " << (int)d.tag << endl;
      ost_ << "\t\t" << "length : " << (int)d.length << endl;
      if(d.tag == service_descriptor::TAG) {
        auto sd = d.as<service_descriptor>();
        ost_ << "\t\t" << "service_type: " << (int)sd->service_type << endl;
        AribToString(
            tmpbuf,
            sd->service_provider_name.data(),
            sd->service_provider_name.size());
        ost_ << "\t\t" << "service_provider_name: " << tmpbuf << endl;
        AribToString(
            tmpbuf,
            sd->service_name.data(),
            sd->service_name.size());
        ost_ << "\t\t" << "service_name: " << tmpbuf << endl;
      }
    }
  }
}

}

#endif
