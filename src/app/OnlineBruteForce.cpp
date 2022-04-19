#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/UserRankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "OnlineBruteForce.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>

class Parameter {
public:
    std::string dataset_name, basic_dir;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake"), "dataset_name")
            ("basic_dir,bd",
             po::value<std::string>(&para.basic_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "basic directory");

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
    const char *dataset_name = para.dataset_name.c_str();
    const char *basic_dir = para.basic_dir.c_str();

    const char *method_name = "OnlineBruteForce";
    spdlog::info("{} dataset_name {}, basic_dir {}", method_name, dataset_name, basic_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    TimeRecord record;
    record.reset();
    OnlineBruteForce::Index obf(data_item, user);
    obf.Preprocess();
    double preprocessed_time = record.get_elapsed_time_second();
    record.reset();
    spdlog::info("finish preprocess");

    vector<int> topk_l{70, 60, 50, 40, 30, 20, 10};
    OnlineBruteForce::RetrievalResult config;
    vector<vector<vector<UserRankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector<vector<UserRankElement>> result_rk = obf.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        string str = config.AddResultConfig(topk, retrieval_time, second_per_query);
        spdlog::info("{}", str);
    }


    spdlog::info("build index time: total {}s", preprocessed_time);
    int n_topk = (int) topk_l.size();
    for (int i = 0; i < n_topk; i++) {
        cout << config.config_l[i] << endl;
        writeRankResult(result_rank_l[i], dataset_name, method_name);
    }
    config.AddPreprocess(preprocessed_time);
    config.writePerformance(dataset_name, method_name);
    return 0;
}