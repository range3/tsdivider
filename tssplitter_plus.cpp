#include <iostream>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>
using namespace std;
namespace po = boost::program_options;

#include "ts_reader.hpp"
#include "context.hpp"
#include "view.hpp"


bool checkProgramOptions(const po::variables_map& vm) {
  if(vm.count("help"))
    return false;

  if(!vm.count("input"))
    return false;

  if(!vm.count("output"))
    return false;

  return true;
}

int main(int argc, char* argv[]) {

  po::options_description desc("options");
  desc.add_options()
    ("help", "produce help message")
    ("input,i", po::value<string>(), "input file (REQUIRED)")
    ("output,o", po::value<string>()->default_value("a.ts"), "output file")
    ("json", "print information by json")
    ("json_prettify", "print information by prettify json")
    ("debug", "print information by debug view")
    ("pat", "print pat")
    ("pmt", "print pmt")
    ("sdt", "print sdt")
    ("tot", "print tot")
    ("all", "print all information")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);


  if(!checkProgramOptions(vm)) {
    cout << desc << endl;
    return 1;
  }

  if(vm.count("input")) {
    cout << "input: " << vm["input"].as<string>() << endl;
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
  view->set_print_pat(vm.count("pat"));
  view->set_print_pmt(vm.count("pmt"));
  view->set_print_sdt(vm.count("sdt"));
  view->set_print_tot(vm.count("tot"));

  try {
    std::ifstream input(
        vm["input"].as<string>(),
        std::ios::binary);
    input.exceptions(std::ifstream::failbit);

    tssp::tsreader reader(input);
    tssp::packet packet;
    tssp::context cxt(std::move(view));
    while(reader.next(packet)) {
      cxt.handle_packet(packet);
    }
  }
  catch(const std::ios_base::failure& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
