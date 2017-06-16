#define BOOST_SCOPE_EXIT_CONFIG_USE_LAMBDAS
#include <iostream>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>
#include <boost/filesystem.hpp>
using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
#include "picojson.h"

#include "ts_reader.hpp"
#include "context.hpp"
#include "view.hpp"
#include "ts_trimmer.hpp"
#include "wrap_around_time_stamp.hpp"


bool checkProgramOptions(const po::variables_map& vm) {
  if(vm.count("help"))
    return false;

  if(!vm.count("input"))
    return false;

  return true;
}

int main(int argc, char* argv[]) {
  po::options_description desc("options");
  desc.add_options()
    ("help", "produce help message")
    ("input,i", po::value<string>(), "input file (REQUIRED)")
    ("output,o", po::value<string>(), "output file")
    ("tmpbuf", po::value<string>(), "temporary buffer file (default: \"${--output}.tmpbuf\"")
    ("json", "print information by json")
    ("json_prettify", "print information by prettify json")
    ("debug", "print information by debug view")
    ("header", "print section header")
    ("pat", "print pat")
    ("pmt", "print pmt")
    ("sdt", "print sdt")
    ("tot", "print tot")
    ("eit", "print eit")
    ("print_if_changed", "print information if section version changed")
    ("packet_num", "print ts packet number")
    ("enable_pmt_separator", po::value<bool>()->default_value(true), "")
    ("enable_eit_separator", po::value<bool>()->default_value(true), "")
    ("trim_threshold", po::value<int64_t>()->default_value(5*60), "(sec)")
    ("overlap_front", po::value<int>()->default_value(1024), "(packet)")
    ("overlap_back", po::value<int>()->default_value(1024), "(packet)")
    ("prettify",       "prettify json")
    ("broadcast_time", "print broadcast time")
    ("program_info",   "print program information")
    ("transport_stream_id", "print transport stream id")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if(!checkProgramOptions(vm)) {
    cout << desc << endl;
    return 1;
  }

  std::unique_ptr<tsd::view> view;
  if(vm.count("json"))
    view.reset(new tsd::json_view());
  else if(vm.count("json_prettify"))
    view.reset(new tsd::json_view(true));
  else if(vm.count("debug"))
    view.reset(new tsd::debug_view());
  else
    view.reset(new tsd::view());
  view->set_print_section_header(vm.count("header"));
  view->set_print_pat(vm.count("pat"));
  view->set_print_pmt(vm.count("pmt"));
  view->set_print_sdt(vm.count("sdt"));
  view->set_print_tot(vm.count("tot"));
  view->set_print_eit(vm.count("eit"));
  view->set_print_if_changed(vm.count("print_if_changed"));
  view->set_print_packet_num(vm.count("packet_num"));

  try {
    std::ifstream input(
        vm["input"].as<string>(),
        std::ios::binary);
    input.exceptions(
        std::ios_base::failbit | std::ios_base::badbit);

    tsd::tsreader reader(input);
    tsd::transport_packet packet;
    tsd::context cxt(std::move(view));

    std::ofstream output;
    output.exceptions(
        std::ios_base::failbit | std::ios_base::badbit);

    std::fstream fbuffer;
    fbuffer.exceptions(
        std::ios_base::failbit | std::ios_base::badbit);
    fs::path fbuffer_path;

    BOOST_SCOPE_EXIT(&fbuffer, &fbuffer_path) {
      if(!fbuffer_path.empty()) {
        fbuffer.exceptions(std::ios_base::goodbit);
        fbuffer.close();
        try {
          if(fs::exists(fbuffer_path))
            fs::remove(fbuffer_path);
        }
        catch(fs::filesystem_error& e) {
          cerr << e.what() << endl;
        }
      }
    };

    if(vm.count("output")) {
      output.open(
          vm["output"].as<string>(),
          std::ios::binary | std::ios::trunc);

      if(vm.count("tmpbuf")) {
        fbuffer_path = (vm["tmpbuf"].as<string>());
      }
      else {
        fbuffer_path =
            std::string(vm["output"].as<string>()) + ".tmpbuf";
      }
      fbuffer.open(
          fbuffer_path.string(),
          std::ios_base::binary |
          std::ios_base::trunc |
          std::ios_base::in |
          std::ios_base::out);

      std::unique_ptr<tsd::ts_trimmer> trimmer(
          new tsd::ts_trimmer(
            output,
            fbuffer,
            vm["trim_threshold"].as<int64_t>(),
            vm["enable_pmt_separator"].as<bool>(),
            vm["enable_eit_separator"].as<bool>(),
            vm["overlap_front"].as<int>(),
            vm["overlap_back"].as<int>()));
      cxt.set_ts_trimmer(std::move(trimmer));
    }

    picojson::object root;
    bool print_program_info_latch = vm.count("program_info");
    bool print_tsid_latch = vm.count("transport_stream_id");

    while(reader.next(packet)) {
      cxt.handle_packet(packet);

      if(print_program_info_latch) {
        if(!cxt.latest_service_descriptors.empty()) {
          picojson::array  program_info;
          char tmpbuf[4096];

          for(auto& kv : cxt.latest_service_descriptors) {
            picojson::object sdo;
            sdo.emplace(
                "program_number",
                picojson::value(static_cast<double>(kv.first)));
            AribToString(
                tmpbuf,
                kv.second.service_name.data(),
                kv.second.service_name.size());
            sdo.emplace(
                "service_name",
                picojson::value(tmpbuf));
            AribToString(
                tmpbuf,
                kv.second.service_provider_name.data(),
                kv.second.service_provider_name.size());
            sdo.emplace(
                "service_provider",
                picojson::value(tmpbuf));
            program_info.emplace_back(std::move(sdo));
          }

          root.emplace(
              "program_info",
              picojson::value(program_info));
          print_program_info_latch = false;
        }
      }

      if(print_tsid_latch) {
        if(cxt.transport_stream_id) {
          root.emplace(
              "transport_stream_id",
              picojson::value(
                static_cast<double>(*cxt.transport_stream_id)));
          print_tsid_latch = false;
        }
      }

      if(!vm.count("output") &&
         !print_program_info_latch &&
         !print_tsid_latch) {
        if(vm.count("broadcast_time")) {
          if(cxt.first_pcr &&
              cxt.latest_pcr &&
              cxt.baseline_pcr &&
              cxt.baseline_time) {
            uint16_t current_pcr = *cxt.latest_pcr;
            auto current_pos = reader.tellg();

            reader.seekg(
                -4092 * tsd::transport_packet::size,
                std::ios_base::end);

            while(reader.next(packet)) {
              cxt.handle_packet(packet);
            }

            if(*cxt.latest_pcr != current_pcr)
              break;
            else {
              reader.seekg(0, std::ios_base::beg);
              cxt.clear();

              while(reader.next(packet)) {
                cxt.handle_packet(packet);
              }
            }
          }
        }
        else {
          break;
        }
      }
    }

    if(vm.count("broadcast_time")) {
      if(cxt.first_pcr &&
          cxt.latest_pcr &&
          cxt.baseline_pcr &&
          cxt.baseline_time) {
        picojson::object broadcast_time_o;
        tsd::wrap_around_time_stamp t0(*cxt.first_pcr);
        tsd::wrap_around_time_stamp tb(*cxt.baseline_pcr);
        tsd::wrap_around_time_stamp t1(*cxt.latest_pcr);
        time_t broadcast_begin =
          *cxt.baseline_time - ((tb - t0) / 90000);
        time_t broadcast_end =
          broadcast_begin + ((t1 - t0) / 90000);
        char tmpbuf[100];
        std::strftime(
            tmpbuf,
            sizeof(tmpbuf),
            "%c %Z",
            std::localtime(&broadcast_begin));
        broadcast_time_o.emplace("begin", picojson::value(tmpbuf));
        std::strftime(
            tmpbuf,
            sizeof(tmpbuf),
            "%c %Z",
            std::localtime(&broadcast_end));
        broadcast_time_o.emplace("end", picojson::value(tmpbuf));
        double duration = (t1 - t0) / 90000.0;
        broadcast_time_o.emplace("duration", picojson::value(duration));

        root.emplace(
            "broadcast_time",
            picojson::value(broadcast_time_o));
      }
    }

    // print information
    if(!root.empty()) {
      cout << picojson::value(root).serialize(static_cast<bool>(vm.count("prettify"))) << endl;
    }
  }
  catch(const std::ios_base::failure& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
