#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "alg/SpaceInnerProduct.hpp"
#include "score_computation/CPUScoreTable.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <filesystem>

using namespace std;
using namespace ReverseMIPS;

int main(int argc, char **argv) {

    uint64_t size = std::filesystem::file_size("../index/index");
    printf("size: %ld\n", size);
    return 0;
}