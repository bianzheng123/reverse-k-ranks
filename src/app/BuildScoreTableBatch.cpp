//
// Created by bianzheng on 2023/2/28.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"

#include "alg/DiskIndex/ReadAll.hpp"
#include "score_computation/ComputeScoreTableBatch.hpp"

#include <spdlog/spdlog.h>
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>
#include <string>

class Parameter {
public:
    std::string dataset_dir, dataset_name, index_dir;
    uint64_t eval_size_gb;
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
            ("index_dir, id",
             po::value<std::string>(&para.index_dir)->default_value("/home/bianzheng/reverse-k-ranks/index"),
             "the directory of the index")
            ("eval_size_gb, esb",
             po::value<uint64_t>(&para.eval_size_gb)->default_value(500),
             "evaluation size, in terms of GB");

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

int ComputeEvalUserSize(const int &n_user, const int &n_data_item, const uint64_t &eval_size_gb) {
    const uint64_t total_size_gb = (u_int64_t) n_user * n_data_item * sizeof(double) / 1024 / 1024 / 1024;
    if (eval_size_gb > total_size_gb) {
        return n_user;
    }
    const uint64_t n_eval_user = eval_size_gb * 1024 * 1024 * 1024 / sizeof(double) / n_data_item;
    assert(n_eval_user < n_user);
    return (int) n_eval_user;
}

int ComputeBatchUser(const int &n_user, const int &n_data_item, const int &vec_dim, const uint64_t &gpu_size_gb) {

    const uint64_t dataset_size = sizeof(double) * (n_user + n_data_item) * vec_dim;
    const uint64_t gpu_size = gpu_size_gb * 1000 * 1000 * 1000;
    const uint64_t batch_n_user = (gpu_size - dataset_size) / (sizeof(double) + sizeof(int)) / n_data_item;
    if (batch_n_user > n_user) {
        return n_user / 10;
    }
    return std::min((int) batch_n_user, 128);
}

void BuildScoreTable(VectorMatrix &user, VectorMatrix &data_item,
                     const char *index_path, const uint64_t &eval_size_gb,
                     double &total_compute_sort_time, double &total_save_index_time,
                     double &estimate_compute_sort_time, double &estimate_save_index_time) {
    const int n_user = user.n_vector_;
    const int n_data_item = data_item.n_vector_;
    const int vec_dim = data_item.vec_dim_;
    const int n_eval_user = ComputeEvalUserSize(n_user, n_data_item, eval_size_gb);
    //build index
    user.vectorNormalize();
    //disk index
    ReadAll disk_ins(n_user, n_data_item, index_path);
    disk_ins.BuildIndexPreprocess();

    total_compute_sort_time = 0;
    total_save_index_time = 0;

    double batch_compute_sort_time = 0;
    double batch_save_index_time = 0;

    const uint64_t batch_n_user = ComputeBatchUser(n_user, n_data_item, vec_dim, 11);
    const int remainder = n_user % batch_n_user == 0 ? 0 : 1;
    const int n_batch = n_user / (int) batch_n_user + remainder;
    spdlog::info("{} user per batch, n_batch {}", batch_n_user, n_batch);
    std::vector<double> batch_distance_l(batch_n_user * n_data_item);

    //Compute Score Table
    ComputeScoreTableBatch cstb(user, data_item, (int) batch_n_user);

    const uint32_t report_every = 30;

    TimeRecord batch_record, operation_record;
    batch_record.reset();

    int n_complete_user = 0;
    for (int batchID = 0; batchID < n_batch; batchID++) {
        operation_record.reset();
        const int start_userID = (int) batch_n_user * batchID;
        if (start_userID > n_eval_user) {
            break;
        }
        const int n_user_batch = n_user - start_userID > batch_n_user ? (int) batch_n_user : n_user - start_userID;
        cstb.ComputeSortItemsBatch(start_userID, n_user_batch, batch_distance_l.data());
        const double tmp_compute_sort_time = operation_record.get_elapsed_time_second();

        operation_record.reset();
        disk_ins.BuildIndexLoop(batch_distance_l.data(), n_user_batch);
        const double tmp_save_index_time = operation_record.get_elapsed_time_second();

        total_compute_sort_time += tmp_compute_sort_time;
        total_save_index_time += tmp_save_index_time;

        batch_compute_sort_time += tmp_compute_sort_time;
        batch_save_index_time += tmp_save_index_time;

        n_complete_user += (int) batch_n_user;

        if (batchID % report_every == 0) {
            spdlog::info(
                    "preprocessed {:.1f}%, Mem: {} Mb, {} s/iter, compute sort time {}s, process index time {}s",
                    batchID / (0.01 * n_batch), get_current_RSS() / 1000000,
                    batch_record.get_elapsed_time_second(),
                    batch_compute_sort_time, batch_save_index_time);
            batch_compute_sort_time = 0;
            batch_save_index_time = 0;
            batch_record.reset();
        }
    }
    estimate_compute_sort_time = total_compute_sort_time / n_complete_user * n_user;
    estimate_save_index_time = total_save_index_time / n_complete_user * n_user;
    cstb.FinishCompute();
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
    const uint64_t eval_size_gb = para.eval_size_gb;
    spdlog::info("evaluation size {}GB", eval_size_gb);

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

    double total_compute_sort_time, total_save_index_time;
    double estimate_compute_sort_time, estimate_save_index_time;

    BuildScoreTable(user, data_item,
                    index_path, eval_size_gb,
                    total_compute_sort_time, total_save_index_time,
                    estimate_compute_sort_time, estimate_save_index_time);

    double build_index_time = record.get_elapsed_time_second();
    spdlog::info("finish preprocess and save the index");

    RetrievalResult config;

    spdlog::info("build index time: total {}s, compute sort time {}s, save index time {}s",
                 build_index_time, total_compute_sort_time, total_save_index_time);
    spdlog::info("estimate compute sort time {}s, estimate save index time {}s",
                 estimate_compute_sort_time, estimate_save_index_time);

    char build_index_info[256];
    sprintf(build_index_info,
            "compute sort time %.3f s, save index time %.3f s, estimate compute sort time %.3f s, estimate save index time %.3f s",
            total_compute_sort_time, total_save_index_time,
            estimate_compute_sort_time, estimate_save_index_time);

    config.AddInfo(build_index_info);
    config.AddBuildIndexTime(build_index_time);

#ifdef USE_GPU
    config.WritePerformance(dataset_name, "BuildIndex", "GPU");
#else
    config.WritePerformance(dataset_name, "BuildIndex", "CPU");
#endif
    return 0;
}