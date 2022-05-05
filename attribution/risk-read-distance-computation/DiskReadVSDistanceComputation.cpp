//
// Created by BianZheng on 2022/3/16.
//

#include "alg/SpaceInnerProduct.hpp"
#include "alg/DiskIndex/RankFromCandidate/GridIndex.hpp"
#include "util/TimeMemory.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <memory>
#include <fstream>
#include <spdlog/spdlog.h>

using namespace std;
using namespace ReverseMIPS;

std::unique_ptr<double[]> GenRandom(const int &n_eval, const int &n_dim) {
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(1.0, 1000.0);
    size_t length = n_eval * n_eval;

    std::unique_ptr<double[]> random_l = make_unique<double[]>(length);

    for (int itemID = 0; itemID < n_eval; itemID++) {
        for (int dim = 0; dim < n_dim; dim++) {
            int id = itemID * n_dim + dim;
            double random = dis(gen);
            random_l[id] = random;
        }
    }
    return random_l;
}

void
BuildWriteIndex(const char *index_path, const double *vecs1, const double *vecs2, const int &n_eval, const int &dim) {

    size_t write_size = n_eval * n_eval;
    std::vector<double> write_array(write_size);
#pragma omp parallel for default(none) shared(n_eval, vecs1, vecs2, write_array, dim)
    for (int xID = 0; xID < n_eval; xID++) {
        const double *x_vecs = vecs1 + xID * dim;
        for (int yID = 0; yID < n_eval; yID++) {
            const double *y_vecs = vecs2 + yID * dim;
            double ip = InnerProduct(x_vecs, y_vecs, dim);
            int id = xID * n_eval + yID;
            write_array[id] = ip;
        }
    }

    //build and write index
    std::ofstream out(index_path, std::ios::binary | std::ios::out);
    if (!out) {
        spdlog::error("error in write result");
    }

    out.write((char *) write_array.data(), n_eval * n_eval * sizeof(double));
}

void AttributionWrite(const std::vector<std::pair<double, double>> &result_l, const std::vector<int> &dim_l) {

    char resPath[256];
    std::sprintf(resPath, "../../result/attribution/DiskReadVSDistanceComputation/result.txt");
    std::ofstream file(resPath);
    if (!file) {
        std::printf("error in write result\n");
    }

    assert(result_l.size() == dim_l.size());
    int size = (int) dim_l.size();

    for (int i = 0; i < size; i++) {
        file << "dimension=" << dim_l[i] << ", computation time " << std::to_string(result_l[i].first)
             << "s, read disk time " << std::to_string(result_l[i].second) << "s" << std::endl;
    }

    file.close();
}

int main(int argc, char **argv) {
    vector<int> dim_l{10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200};
    const int n_eval = 10000;
    const char *index_path = "../../index/DiskRead.index";
    spdlog::info("DiskReadVSDistanceComputation");

    vector<pair<double, double>> result_l;

    for (const int &dimension: dim_l) {
        std::unique_ptr<double[]> vecs1 = GenRandom(n_eval, dimension);
        std::unique_ptr<double[]> vecs2 = GenRandom(n_eval, dimension);
        size_t length = (size_t) n_eval * n_eval;

        //compute the inner product
        TimeRecord record;
        record.reset();
        double comp_res = 0;
        for (int xID = 0; xID < n_eval; xID++) {
            double *x_vecs = vecs1.get() + xID * dimension;
            for (int yID = 0; yID < n_eval; yID++) {
                double *y_vecs = vecs2.get() + yID * dimension;
                double ip = InnerProduct(x_vecs, y_vecs, dimension);
                comp_res += ip;
            }
        }
        double comp_time = record.get_elapsed_time_second();

        // write the index into the disk
        BuildWriteIndex(index_path, vecs1.get(), vecs2.get(), n_eval, dimension);
        std::vector<double> read_array(length);
        std::ifstream in(index_path, std::ios::binary | std::ios::in);
        size_t read_size = (size_t) n_eval * n_eval * sizeof(double);

        record.reset();
        in.read((char *) read_array.data(), read_size);
        double read_disk_time = record.get_elapsed_time_second();

        double disk_res = 0;
        for (int xID = 0; xID < n_eval; xID++) {
            for (int yID = 0; yID < n_eval; yID++) {
                disk_res += read_array[xID * n_eval + yID];
            }
        }

        //use grid index
        VectorMatrix user, data_item;
        user.init(vecs1, n_eval, dimension);
        data_item.init(vecs2, n_eval, dimension);

        GridIndex gridIndex;
        gridIndex.Preprocess(user, data_item, 2);
        std::vector<int> item_cand_l(n_eval);
        for (int itemID = 0; itemID < n_eval; itemID++) {
            item_cand_l[itemID] = itemID;
        }
        std::vector<std::pair<double, double>> IPbound_l(n_eval);

        double grid_lb = 0;
        double grid_ub = 0;
        record.reset();
        for (int userID = 0; userID < n_eval; userID++) {
            gridIndex.CalcIPBound(item_cand_l, userID, IPbound_l);
//            for (int itemID = 0; itemID < n_eval; itemID++) {
//                grid_lb += IPbound_l[itemID].first;
//                grid_ub += IPbound_l[itemID].second;
//            }
        }
        double grid_time = record.get_elapsed_time_second();
        grid_lb /= (n_eval * n_eval);
        grid_ub /= (n_eval * n_eval);


        spdlog::info(
                "dimension {}, computation time {}s, read disk time {}s, grid time {}s",
                dimension, comp_time, read_disk_time, grid_time);
        spdlog::info("compute res {}, disk read res {}, grid lb res {}, grid ub res {}",
                     comp_res, disk_res, grid_lb, grid_ub);
//        assert(grid_lb <= comp_res && comp_res <= grid_ub);
        result_l.emplace_back(comp_time, read_disk_time);
    }
    AttributionWrite(result_l, dim_l);

    return 0;
}
