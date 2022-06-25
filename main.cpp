#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "signature.h"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    try {
        std::string inDataFile;
        std::string outSignatureFile;
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("signature", "produce signature for given file")("infile", po::value(&inDataFile), "input file for which signature shall be calculated")("outfile", po::value(&outSignatureFile), "output file to which signature shall be stored");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("signature")) {
            std::cout << "signature option was chosen\n";
            if (!vm.count("infile")) {
                std::cout << "--infile is required with --signature\n";
                return -1;
            }

            filediff::Signature signature(inDataFile, filediff::Signature::InputFileType::BASIS);
            if (outSignatureFile != "") {
                std::ofstream outStream { outSignatureFile, std::ios::binary };
                signature.Serialize(outStream);
            } else {
                std::ostream outStream { std::cout.rdbuf() };
                signature.Serialize(outStream);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Exception of unknown type!\n";
    }
    return 0;
}
