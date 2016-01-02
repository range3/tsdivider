#include <iostream>
#include <string>
#include <fstream>
#include <boost/program_options.hpp>
using namespace std;
namespace po = boost::program_options;

#include "ts_reader.hpp"


bool checkProgramOptions(const po::variables_map& vm) {
  if(vm.count("help"))
    return false;

  if(!vm.count("input"))
    return false;

  if(!vm.count("output"))
    return false;

  return true;
}

class stream_reader {
public:
  stream_reader(std::istream& input) : input_(input) {}

  void read_all() {
    input_.exceptions(std::istream::badbit);

    char buf[1024] = {};
    while(input_) {
      input_.read(buf, 1024);
      cout << input_.gcount() << " " << endl;
    }

    if(!input_.eof())
      input_.exceptions(std::istream::failbit);
  }

private:
  std::istream& input_;
};

int main(int argc, char* argv[]) {

  po::options_description desc("options");
  desc.add_options()
    ("help", "produce help message")
    ("input,i", po::value<string>(), "input file (REQUIRED)")
    ("output,o", po::value<string>()->default_value("a.ts"), "output file")
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

  try {
    std::ifstream input(
        vm["input"].as<string>(),
        std::ios::binary);
    input.exceptions(std::ifstream::failbit);

    //stream_reader sr(input);
    //sr.read_all();

    tssp::tsreader reader(input);
    tssp::packet packet;
    while(reader.next(packet)) {
      cout << static_cast<unsigned int>(packet.pid()) << " " << flush;
      //cout << static_cast<int>(packet.continuity_index()) << " " << flush;
    }
  }
  catch(const std::ios_base::failure& e) {
    cerr << "error: " << e.what() << endl;
    return 1;
  }

  return 0;
}
