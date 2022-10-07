//
// Created by BianZheng on 2022/9/20.
//

//
// Created by bianzheng on 2022/4/29.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"

#include "alg/DiskIndex/ReadAll.hpp"
#include "score_computation/ComputeScoreTable.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, index_dir;
};

void LoadOptions(int argc, char **argv, Parameter &para) {
    namespace po = boost::program_options;

    po::options_description opts("Allowed options");
    opts.add_options()
            ("help,h", "help info")
            ("dataset_dir,dd",
             po::value<std::string>(&para.dataset_dir)->default_value("/home/bianzheng/Dataset/ReverseMIPS"),
             "the basic directory of dataset")
            ("dataset_name, ds", po::value<std::string>(&para.dataset_name)->default_value("fake-normal"),
             "dataset_name")
            ("index_dir, id", po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index");

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

void BuildScoreTable(VectorMatrix &user, VectorMatrix &data_item,
                     const char *index_path,
                     double &total_compute_time, double &total_sort_time, double &total_process_index_time) {
    const int n_user = user.n_vector_;
    const int n_data_item = data_item.n_vector_;
    //build index
    user.vectorNormalize();
    //disk index
    ReadAll disk_ins(n_user, n_data_item, index_path);
    disk_ins.BuildIndexPreprocess();

    //Compute Score Table
    ComputeScoreTable cst(user, data_item);
    cst.compute_time_ = 0;
    cst.sort_time_ = 0;

    TimeRecord batch_record, batch_process_index_record;
    batch_record.reset();

    total_compute_time = 0;
    total_sort_time = 0;
    total_process_index_time = 0;

    double batch_compute_time = 0;
    double batch_sort_time = 0;
    double batch_process_index_time = 0;

    std::vector<DistancePair> distance_l(n_data_item);
    for (int userID = 0; userID < n_user; userID++) {
        cst.ComputeSortItems(userID, distance_l.data());

        batch_process_index_record.reset();
        disk_ins.BuildIndexLoop(distance_l.data());
        const double tmp_process_index_time = batch_process_index_record.get_elapsed_time_second();
        const double tmp_compute_time = cst.compute_time_;
        const double tmp_sort_time = cst.sort_time_;

        total_compute_time += tmp_compute_time;
        total_sort_time += tmp_sort_time;
        total_process_index_time += tmp_process_index_time;

        batch_compute_time += tmp_compute_time;
        batch_sort_time += tmp_sort_time;
        batch_process_index_time += tmp_process_index_time;

        cst.compute_time_ = 0;
        cst.sort_time_ = 0;

        if (userID % cst.report_every_ == 0) {
            spdlog::info(
                    "preprocessed {:.1f}%, Mem: {} Mb, {} s/iter, compute time {}s, sort time {}s, process index time {}s",
                    userID / (0.01 * n_user), get_current_RSS() / 1000000,
                    batch_record.get_elapsed_time_second(),
                    batch_compute_time, batch_sort_time, batch_process_index_time);
            batch_compute_time = 0;
            batch_sort_time = 0;
            batch_process_index_time = 0;
            batch_record.reset();
        }
    }
    cst.FinishCompute();
    disk_ins.FinishBuildIndex();
}


int main(int argc, char **argv) {
    Parameter para;
    LoadOptions(argc, argv, para);
    const char *dataset_name = para.dataset_name.c_str();
    const char *dataset_dir = para.dataset_dir.c_str();
    string index_dir = para.index_dir;
    spdlog::info("dataset_name {}, dataset_dir {}", dataset_name, dataset_dir);
    spdlog::info("index_dir {}", index_dir);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<VectorMatrix> data = readData(dataset_dir, dataset_name, n_data_item, n_query_item, n_user,
                                         vec_dim);
    VectorMatrix &user = data[0];
    VectorMatrix &data_item = data[1];
    VectorMatrix &query_item = data[2];
    spdlog::info("n_data_item {}, n_query_item {}, n_user {}, vec_dim {}", n_data_item, n_query_item, n_user, vec_dim);

    char index_path[256];
    sprintf(index_path, "%s/%s.index", index_dir.c_str(), dataset_name);

    TimeRecord record;
    record.reset();
    char parameter_name[256] = "";

    double total_compute_time, total_sort_time, total_process_index_time;

    BuildScoreTable(user, data_item,
                    index_path,
                    total_compute_time, total_sort_time, total_process_index_time);

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    RetrievalResult config;

    spdlog::info("build index time: total {}s", build_index_time);

    char build_index_info[256];
    sprintf(build_index_info, "compute time %.3f s, sort time %.3f s, process index time %.3f s",
            total_compute_time, total_sort_time, total_process_index_time);

    config.AddBuildIndexInfo(build_index_info);
    config.AddBuildIndexTime(build_index_time);
    config.WritePerformance(dataset_name, "BuildIndex", parameter_name);
    return 0;
}