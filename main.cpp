#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "delta.h"
#include "signature.h"

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    try {
        std::string inDataFile, outSignatureFile, sigfile, newdata;
        po::options_description desc("Allowed options");
        desc.add_options()("help", "produce help message")("signature", "produce signature for given file")("infile", po::value(&inDataFile), "input file for which signature shall be calculated")("outfile", po::value(&outSignatureFile), "output file to which signature shall be stored")("delta", "calculates delta based on given sigfile and newdata files")("sigfile", po::value(&sigfile), "signature file calculated for base data file")("newdata", po::value(&newdata), "data file to be compared");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("signature") && vm.count("delta")) {
            std::cout << "signature and delta should not be called at once!\n";
            return -1;
        }

        if (vm.count("signature")) {
            if (inDataFile == "") {
                std::cout << "--infile is required with --signature\n";
                return -2;
            }

            filediff::Signature signature(inDataFile, filediff::Signature::InputFileType::BASIS);
            if (outSignatureFile != "") {
                std::ofstream outStream { outSignatureFile, std::ios::binary };
                signature.Serialize(outStream);
            } else {
                std::ostream outStream { std::cout.rdbuf() };
                signature.Serialize(outStream);
            }
        } else if (vm.count("delta")) {
            if (sigfile == "") {
                std::cout << "--sigfile name is required with --delta\n";
                return -3;
            }
            if (newdata == "") {
                std::cout << "--newdata file name is required with --delta\n";
                return -4;
            }

            filediff::Delta delta { sigfile, newdata };
            delta.Calculate();
            if (delta.IsChanged()) {
                std::ostream ostream { std::cout.rdbuf() };
                delta.SerializeDelta(ostream);
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
