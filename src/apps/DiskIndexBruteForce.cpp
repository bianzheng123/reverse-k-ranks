//
// Created by BianZheng on 2021/12/22.
//

#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/RankElement.hpp"
#include "struct/VectorMatrix.hpp"
#include "DiskIndexBruteForce.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <map>

//预处理时不做任何动作, 在线计算全部的向量, 然后返回最大的k个rank

using namespace std;
using namespace ReverseMIPS;

class RetrievalResult {
public:
    //unit: second
    double total_time, read_disk_time, inner_product_time, binary_search_time, second_per_query;
    int topk;

    inline RetrievalResult(double total_time, double read_disk_time, double inner_product_time,
                           double binary_search_time, double second_per_query, int topk) {
        this->total_time = total_time;
        this->read_disk_time = read_disk_time;
        this->inner_product_time = inner_product_time;
        this->binary_search_time = binary_search_time;
        this->second_per_query = second_per_query;

        this->topk = topk;
    }

    void AddMap(map<string, string> &performance_m) {
        char buff[256];
        sprintf(buff, "top%d total retrieval time", topk);
        string str1(buff);
        performance_m.emplace(str1, double2string(total_time));

        sprintf(buff, "top%d retrieval read disk time", topk);
        string str2(buff);
        performance_m.emplace(str2, double2string(read_disk_time));

        sprintf(buff, "top%d retrieval inner product time", topk);
        string str3(buff);
        performance_m.emplace(str3, double2string(inner_product_time));

        sprintf(buff, "top%d retrieval binary search time", topk);
        string str4(buff);
        performance_m.emplace(str4, double2string(binary_search_time));

        sprintf(buff, "top%d second per query time", topk);
        string str5(buff);
        performance_m.emplace(str5, double2string(second_per_query));
    }

    [[nodiscard]] std::string ToString() const {
        char arr[256];
        sprintf(arr,
                "top%d retrieval time:\n\ttotal %.3fs, read disk %.3fs\n\tinner product %.3fs, binary search %.3fs, million second per query %.3fms",
                topk, total_time, read_disk_time, inner_product_time, binary_search_time, second_per_query * 1000);
        std::string str(arr);
        return str;
    }

};


int main(int argc, char **argv) {
    if (!(argc == 2 or argc == 3)) {
        cout << argv[0] << " dataset_name [basic_dir]" << endl;
        return 0;
    }
    const char *dataset_name = argv[1];
    const char *basic_dir = "/run/media/hdd/ReverseMIPS";
    if (argc == 3) {
        basic_dir = argv[2];
    }
    printf("DiskIndexBruteForce dataset_name %s, basic_dir %s\n", dataset_name, basic_dir);

    double build_index_calculation_time, total_build_index_time;
    char index_path[256];
    sprintf(index_path, "../index/%s.bfi", dataset_name);

    int n_data_item, n_query_item, n_user, vec_dim;
    vector<unique_ptr<double[]>> data = readData(basic_dir, dataset_name, n_data_item, n_query_item, n_user,
                                                 vec_dim);
    double *data_item_ptr = data[0].get();
    double *user_ptr = data[1].get();
    double *query_item_ptr = data[2].get();
    printf("n_data_item %d, n_query_item %d, n_user %d, vec_dim %d\n", n_data_item, n_query_item, n_user, vec_dim);

    VectorMatrix data_item, user, query_item;
    data_item.init(data_item_ptr, n_data_item, vec_dim);
    user.init(user_ptr, n_user, vec_dim);
    query_item.init(query_item_ptr, n_query_item, vec_dim);

    TimeRecord record;
    record.reset();
    build_index_calculation_time = BuildSaveIndex(data_item, user, index_path);
    total_build_index_time = record.get_elapsed_time_second();
    printf("finish preprocess and save the index\n");

    DiskIndexBruteForce dibf(index_path, n_data_item, user);

    vector<int> topk_l{10, 20, 30, 40, 50};
    vector<RetrievalResult> retrieval_res_l;
    vector<vector<vector<RankElement>>> result_rank_l;
    for (int topk: topk_l) {
        record.reset();
        vector<vector<RankElement>> result_rk = dibf.Retrieval(query_item, topk);

        double retrieval_time = record.get_elapsed_time_second();
        double read_disk_time = dibf.read_disk_time_;
        double inner_product_time = dibf.inner_product_time_;
        double binary_search_time = dibf.binary_search_time_;
        double second_per_query = retrieval_time / n_query_item;

        result_rank_l.emplace_back(result_rk);
        retrieval_res_l.emplace_back(retrieval_time, read_disk_time, inner_product_time, binary_search_time,
                                     second_per_query, topk);
    }

    printf("build index time: total %.3fs, inner product calculation %.3fs\n",
           total_build_index_time, build_index_calculation_time);
    int n_topk = (int) topk_l.size();

    for (int i = 0; i < n_topk; i++) {
        cout << retrieval_res_l[i].ToString() << endl;
        writeRank(result_rank_l[i], dataset_name, "DiskIndexBruteForce");
    }

    map<string, string> performance_m;
    performance_m.emplace("build index total time", double2string(total_build_index_time));
    performance_m.emplace("build index calculation time", double2string(build_index_calculation_time));
    for (int i = 0; i < n_topk; i++) {
        retrieval_res_l[i].AddMap(performance_m);
    }
    writePerformance(dataset_name, "DiskIndexBruteForce", performance_m);

    return 0;
}