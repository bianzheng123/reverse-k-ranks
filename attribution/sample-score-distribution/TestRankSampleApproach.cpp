//
// Created by BianZheng on 2022/10/22.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "alg/RankBoundRefinement/RankSearch.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    int n_sample, n_data_item;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            // memory index parameter
            ("n_data_item, ns", po::value<int>(&para.n_data_item)->default_value(500),
             "number of data item")
            ("n_sample, ns", po::value<int>(&para.n_sample)->default_value(20),
             "number of sample of a rank bound");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << opts << std::endl;
        exit(0);
    }
}

using namespace std;
using namespace ReverseMIPS;


int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    spdlog::info("TestRankSampleApproach");

    const int n_data_item = para.n_data_item;
    const int n_sample = para.n_sample;

    spdlog::info("n_data_item {}, n_sample {}", n_data_item, n_sample);

    TimeRecord record;
    record.reset();

    //rank search
    RankSearch rank_ins(n_sample, n_data_item, 1);

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");
    return 0;
}