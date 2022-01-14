//
// Created by BianZheng on 2021/12/27.
//

#ifndef REVERSE_KRANKS_SAMPLEINDEX_HPP
#define REVERSE_KRANKS_SAMPLEINDEX_HPP

#include "struct/VectorMatrix.hpp"
#include "util/TimeMemory.hpp"
#include "util/StringUtil.hpp"
#include "util/SpaceInnerProduct.hpp"
#include "struct/SetVector.hpp"
#include "struct/DistancePair.hpp"
#include <fstream>

namespace ReverseMIPS {
    int n_index_rank = -1;

    std::vector<int> GetMergeIndices(const char *dataset_name, int &n_merge_user) {
        char dir[256];
        sprintf(dir, "/home/bianzheng/Reverse-kRanks/index/RankInterval-%s/user_idx.txt", dataset_name);
        std::ifstream in(dir);
        int n_user;

        std::string line;
        getline(in, line);
        std::vector<std::string> arr_str = split(line, ',');
        n_user = atoi(arr_str[0].c_str());
        n_merge_user = atoi(arr_str[1].c_str());

        std::vector<int> merge_idx(n_user);
        for (int i = 0; i < n_user; i++) {
            getline(in, line);
            merge_idx[i] = atoi(line.c_str());
        }
        return merge_idx;

    }

    void BuildIndex(VectorMatrix user, VectorMatrix data_item, const char *dataset_name, double &time) {
        /*
         * 首先进行merge用户, 然后建立索引, 根据指定的方向进行merge
         */
        int n_merge_user;
        std::vector<int> merge_idx = GetMergeIndices(dataset_name, n_merge_user);

        int n_user = user.n_vector_;
        int n_data_item = data_item.n_vector_;
        int vec_dim = user.vec_dim_;

        n_index_rank = std::max(n_data_item / 10, 100);

        //confirm the specific rank, start from 1
        std::vector<int> rank_dim_idx_l(n_index_rank);
        int n_interval_rank = n_data_item / n_index_rank;
        for (int i = 0; i < n_index_rank; i++) {
            rank_dim_idx_l[i] = (i + 1) * n_interval_rank;
        }

        std::vector<DistancePair> distance_table(n_user * n_index_rank);
        std::vector<SetVector> interval_arr(n_merge_user, SetVector(n_index_rank));

        std::vector<DistancePair> distance_cache(n_data_item);
        for (int userID = 0; userID < n_user; userID++) {
            for (int itemID = 0; itemID < n_data_item; itemID++) {
                float ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                distance_cache[itemID] = DistancePair(ip, itemID);
            }
            std::sort(distance_cache.begin(), distance_cache.end(), std::greater<DistancePair>());


            int interval_arr_dim = n_index_rank + 1;
            std::vector<std::vector<int>> rank_table(interval_arr_dim, std::vector<int>());
            for (int i = 0; i < n_index_rank; i++) {
                distance_table[userID * n_index_rank + i] = distance_cache[rank_dim_idx_l[i]];
                int n_ele_interval, base_idx;
                if (i == 0) {
                    base_idx = 0;
                    n_ele_interval = rank_dim_idx_l[i] - 1;
                } else {
                    n_ele_interval = rank_dim_idx_l[i] - rank_dim_idx_l[i - 1] - 1;
                    base_idx = rank_dim_idx_l[i];
                }

                for (int j = 0; j < n_ele_interval; j++) {
                    rank_table[i].emplace_back(distance_cache[base_idx + j].ID_);
                }
            }
            {
                int base_idx = rank_dim_idx_l[n_index_rank - 1] + 1;
                int n_ele_interval = n_data_item - rank_dim_idx_l[n_index_rank - 1] - 1;
                for (int j = 0; j < n_ele_interval; j++) {
                    rank_table[n_index_rank].emplace_back(distance_cache[base_idx + j].ID_);
                }
            }
            SetVector tmp(rank_table, interval_arr_dim);

            interval_arr[merge_idx[userID]].Merge(tmp);

        }


        //需要返回的东西, distance_table, interval_arr, rank_dim_idx_l


        //test
        printf("rank_dim_idx_l\n");
        for(int i=0;i<rank_dim_idx_l.size();i++){
            printf("%d ", rank_dim_idx_l[i]);
        }
        printf("\n");




    }
}

#endif //REVERSE_KRANKS_SAMPLEINDEX_HPP
