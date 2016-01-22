#include <iostream>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>
using namespace std;
namespace po = boost::program_options;
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
    ("broadcast_time", "print broadcast time")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if(!checkProgramOptions(vm)) {
    cout << desc << endl;
    return 1;
  }

  std::unique_ptr<tssp::view> view;
  if(vm.count("json"))
    view.reset(new tssp::json_view());
  else if(vm.count("json_prettify"))
    view.reset(new tssp::json_view(true));
  else if(vm.count("debug"))
    view.reset(new tssp::debug_view());
  else
    view.reset(new tssp::view());
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
    input.exceptions(std::ifstream::failbit);

    tssp::tsreader reader(input);
    tssp::transport_packet packet;
    tssp::context cxt(std::move(view));

    std::ofstream output;
    output.exceptions(
        std::ofstream::failbit | std::ofstream::badbit);
    if(vm.count("output")) {
      output.open(
          vm["output"].as<string>(),
          std::ios::binary | std::ios::trunc);

      std::unique_ptr<tssp::ts_trimmer> trimmer(
          new tssp::ts_trimmer(
            output,
            vm["trim_threshold"].as<int64_t>(),
            vm["enable_pmt_separator"].as<bool>(),
            vm["enable_eit_separator"].as<bool>(),
            vm["overlap_front"].as<int>(),
            vm["overlap_back"].as<int>()));
      cxt.set_ts_trimmer(std::move(trimmer));
    }

    while(reader.next(packet)) {
      cxt.handle_packet(packet);
    }

    // print information
    picojson::object root;
    if(vm.count("broadcast_time")) {
      picojson::object broadcast_time_o;
      if(cxt.first_pcr &&
         cxt.latest_pcr &&
         cxt.baseline_pcr &&
         cxt.baseline_time) {
        tssp::wrap_around_time_stamp t0(*cxt.first_pcr);
        tssp::wrap_around_time_stamp tb(*cxt.baseline_pcr);
        tssp::wrap_around_time_stamp t1(*cxt.latest_pcr);
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
      }
      root.emplace(
          "broadcast_time", picojson::value(broadcast_time_o));
    }
    if(!root.empty()) {
      cout << picojson::value(root).serialize(true) << endl;
    }
  }
  catch(const std::ios_base::failure& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
