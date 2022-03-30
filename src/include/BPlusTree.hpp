//
// Created by BianZheng on 2022/3/28.
//

#ifndef REVERSE_KRANKS_BPLUSTREE_HPP
#define REVERSE_KRANKS_BPLUSTREE_HPP

#include "alg/SpaceInnerProduct.hpp"
#include "util/VectorIO.hpp"
#include "util/TimeMemory.hpp"
#include "util/FileIO.hpp"
#include "struct/VectorMatrix.hpp"
#include "struct/MethodBase.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>
#include <spdlog/spdlog.h>

namespace ReverseMIPS::BPlusTree {
    class RetrievalResult : public RetrievalResultBase {
    public:
        //unit: second
        //double total_time, read_disk_time, inner_product_time, binary_search_time, second_per_query;
        //int topk;

        inline RetrievalResult() {
        }

        void AddPreprocess(double build_index_time) {
            char buff[1024];
            sprintf(buff, "build index time %.3f", build_index_time);
            std::string str(buff);
            this->config_l.emplace_back(str);
        }

        std::string AddResultConfig(int topk, double total_time, double read_disk_time, double inner_product_time,
                                    double binary_search_time, double second_per_query) {
            char buff[1024];

            sprintf(buff,
                    "top%d retrieval time:\n\ttotal %.3fs, read disk %.3fs\n\tinner product %.3fs, binary search %.3fs, million second per query %.3fms",
                    topk, total_time, read_disk_time, inner_product_time,
                    binary_search_time, second_per_query);
            std::string str(buff);
            this->config_l.emplace_back(str);
            return str;
        }

    };

    class TreeIndex {
    public:
        /**The implementation doesn't have the node, it samples the index of each column**/
        //meta information
        std::vector<int> n_ip_layer_l_;//height_, top to bottom
        std::vector<int> n_ip_cum_layer_l_; // height_, top to bottom
        std::vector<size_t> n_ip_cum_layer_byte_l_;// height_, top to bottom
        size_t tree_size_byte_;
        int n_data_item_, sample_every_;
        int n_layer_, n_ip_tree_;

        //IO
        const char *index_path_;
        std::ofstream out_stream_;
        std::ifstream in_stream_;
        TimeRecord read_disk_record_, binary_search_record_;
        size_t index_size_;

        //Retrieval
        std::vector<double> item_cache_;

        inline TreeIndex() = default;

        inline TreeIndex(const char *index_path, const int &n_data_item, const int &sample_every) {
            this->index_path_ = index_path;
            out_stream_ = std::ofstream(index_path, std::ios::binary | std::ios::out);
            if (!out_stream_) {
                spdlog::error("error in write result");
                exit(-1);
            }
            if (sample_every > n_data_item) {
                spdlog::error("sample number is too large, program exit");
                exit(-1);
            }

            this->n_data_item_ = n_data_item;
            this->sample_every_ = sample_every;
            this->item_cache_.resize(sample_every_);

            GetMetaInfo();
        }

        void GetMetaInfo() {
            n_layer_ = 1;
            n_ip_layer_l_.clear();
            n_ip_cum_layer_l_.clear();
            n_ip_cum_layer_byte_l_.clear();

            int tmp_n_ip_layer = n_data_item_;
            // append the data from the bottom to the top
            n_ip_layer_l_.emplace_back(tmp_n_ip_layer);
            while (tmp_n_ip_layer > sample_every_) {
                int n_ip_layer = tmp_n_ip_layer / sample_every_;
                tmp_n_ip_layer = n_ip_layer;
                n_ip_layer_l_.emplace_back(tmp_n_ip_layer);
                n_layer_++;
            }

            std::reverse(n_ip_layer_l_.begin(), n_ip_layer_l_.end());

            assert(n_layer_ == n_ip_layer_l_.size());

            n_ip_cum_layer_l_.emplace_back(n_ip_layer_l_[0]);
            n_ip_cum_layer_byte_l_.emplace_back(n_ip_layer_l_[0] * sizeof(double));

            for (int layer = 1; layer < n_layer_; layer++) {
                int n_ip_cum_layer = n_ip_cum_layer_l_[layer - 1] + n_ip_layer_l_[layer];
                n_ip_cum_layer_l_.emplace_back(n_ip_cum_layer);
                n_ip_cum_layer_byte_l_.emplace_back(n_ip_cum_layer * sizeof(double));
            }
            n_ip_tree_ = n_ip_cum_layer_l_[n_layer_ - 1];
            tree_size_byte_ = n_ip_cum_layer_byte_l_[n_layer_ - 1];

            int test_n_node = 0;
            for (int layer = 0; layer < n_layer_; layer++) {
                test_n_node += n_ip_layer_l_[layer];
            }
            assert(test_n_node == n_ip_cum_layer_l_[n_layer_ - 1]);
            assert(test_n_node * sizeof(double) == n_ip_cum_layer_byte_l_[n_layer_ - 1]);
            assert(n_ip_layer_l_.size() == n_layer_ && n_ip_cum_layer_l_.size() == n_layer_ &&
                   n_ip_cum_layer_byte_l_.size() == n_layer_);

            printf("n_ip_layer_l\n");
            for (int layer = 0; layer < n_layer_; layer++) {
                printf("%d ", n_ip_layer_l_[layer]);
            }
            printf("\n");

            printf("n_ip_cum_layer_l\n");
            for (int layer = 0; layer < n_layer_; layer++) {
                printf("%d ", n_ip_cum_layer_l_[layer]);
            }
            printf("\n");

            printf("n_ip_cum_layer_byte_l\n");
            for (int layer = 0; layer < n_layer_; layer++) {
                printf("%ld ", n_ip_cum_layer_byte_l_[layer]);
            }
            printf("\n");
        }

        void BuildWriteTree(const double *IP_vecs) {
            //node from the bottom to the top, idx 0 is the bottom
            std::vector<std::vector<double>> ip_layer_l;

            //build the TreeNode from the bottom
            {
                std::vector<double> ip_l(n_data_item_);
                std::memcpy(ip_l.data(), IP_vecs, n_data_item_ * sizeof(double));
                ip_layer_l.push_back(ip_l);
                assert(ip_l.size() == n_ip_layer_l_[n_layer_ - 1]);
            }

            //get the IP for each layer
            for (int layer = n_layer_ - 2; layer >= 0; layer--) {
                int n_ip_layer = n_ip_layer_l_[layer];
                std::vector<double> &prev_ip_l = ip_layer_l[n_layer_ - 2 - layer];
                const int prev_size = (int) prev_ip_l.size();
                //sample
                std::vector<double> ip_l;
                for (int ipID = sample_every_ - 1; ipID < prev_size; ipID += sample_every_) {
                    ip_l.emplace_back(prev_ip_l[ipID]);
                }
                assert(ip_l.size() == n_ip_layer);
                ip_layer_l.push_back(ip_l);
            }

            assert(ip_layer_l.size() == n_layer_);

            // write the ip_layer to the disk
            for (int layer = n_layer_ - 1; layer >= 0; layer--) {
                std::vector<double> ip_l = ip_layer_l[layer];
                assert(ip_l.size() == n_ip_layer_l_[n_layer_ - 1 - layer]);
                out_stream_.write((char *) ip_l.data(), sizeof(double) * ip_l.size());
            }

        }

        void ReadDiskPreprocess() {
            in_stream_ = std::ifstream(this->index_path_, std::ios::binary | std::ios::in);
            if (!in_stream_) {
                spdlog::error("error in reading index");
                exit(-1);
            }
            in_stream_.seekg(0, std::ios::end);
            std::ios::pos_type ss = in_stream_.tellg();
            in_stream_.seekg(0, std::ios::beg);
            size_t fsize = (size_t) ss;
            index_size_ = fsize;
//            printf("index size %ld byte\n", fsize);
        }

        int Retrieval(const double &queryIP, const int &userID, double &read_disk_time, double &binary_search_time,
                      const int &queryID) {
            size_t base_offset = userID * tree_size_byte_;
            assert(in_stream_);
            int rank = 0;
            read_disk_time = 0;
            binary_search_time = 0;

            for (int layer = 0; layer < n_layer_; layer++) {
                int base_rank = rank * sample_every_;
                size_t layer_offset = layer == 0 ? 0 : n_ip_cum_layer_byte_l_[layer - 1];
                size_t block_offset = base_rank * sizeof(double);
                size_t offset = base_offset + layer_offset + block_offset;

                size_t normal_size = sample_every_ * sizeof(double);
                size_t remain_size = n_ip_cum_layer_byte_l_[layer] - block_offset - layer_offset;
                size_t read_byte = std::min(remain_size, normal_size);
                assert(read_byte % sizeof(double) == 0);

//                if (queryID == 2 && userID == 214) {
//                    printf("queryIO %d, userID %d, layer %d, rank %d, offset %ld, filesize %ld\n", queryID, userID,
//                           layer, rank, offset, index_size_);
//                }

//                printf("queryID %d, userID %d\n", queryID, userID);
                assert(offset + read_byte <= index_size_);
                read_disk_record_.reset();
                in_stream_.seekg((int64_t) offset, std::ios::beg);
                in_stream_.read((char *) item_cache_.data(), (int64_t) read_byte);
                read_disk_time += read_disk_record_.get_elapsed_time_second();

                binary_search_record_.reset();
                int arr_size = (int) (read_byte / sizeof(double));
                rank = BinarySearch(queryIP, base_rank, arr_size);

//                if (queryID == 2 && userID == 214) {
//                    printf("after rank %d, arr_size %d\n", rank, arr_size);
//                    printf("%.3f %.3f %.3f %.3f %.3f, IP %.3f\n", item_cache_[0], item_cache_[1], item_cache_[2],
//                           item_cache_[3], item_cache_[4], queryIP);
//                }
//                printf("queryID %d, userID %d\n", queryID, userID);
                assert(0 <= rank && rank <= n_ip_layer_l_[layer]);
                binary_search_time += binary_search_record_.get_elapsed_time_second();
            }
//            printf("queryID %d, userID %d, pos %ld\n", queryID, userID, in_stream_.tellg());
            assert(in_stream_);
            assert(rank >= 0);
            return rank + 1;
        }

        int BinarySearch(const double &queryIP, const int &base_rank, const int &arr_size) const {
            const double *ip_vecs = item_cache_.data();

            const double *lb_vecs = std::lower_bound(ip_vecs, ip_vecs + arr_size, queryIP,
                                                     [](const double &arrIP, double queryIP) {
                                                         return arrIP > queryIP;
                                                     });
            int offset_rank = (int) (lb_vecs - ip_vecs);

            return offset_rank + base_rank;
        }

    };

    class Index : public BaseIndex {
        void ResetTimer() {
            read_disk_time_ = 0;
            inner_product_time_ = 0;
            binary_search_time_ = 0;
        }

    public:
        TreeIndex tree_ins_;
        VectorMatrix user_;
        int vec_dim_, n_data_item_;
        double read_disk_time_, inner_product_time_, binary_search_time_;
        TimeRecord read_disk_record_, inner_product_record_, binary_search_record_;
        const int report_query_every_ = 5;

        Index() {}

        Index(TreeIndex &tree_ins, const int n_data_item, VectorMatrix &user) {
            this->tree_ins_ = std::move(tree_ins);
            this->vec_dim_ = user.vec_dim_;
            this->user_ = std::move(user);
            this->n_data_item_ = n_data_item;
        }

        std::vector<std::vector<UserRankElement>> Retrieval(VectorMatrix &query_item, const int &topk) override {
            ResetTimer();

            if (topk > user_.n_vector_) {
                spdlog::error("top-k is too large, program exit");
                exit(-1);
            }

            tree_ins_.ReadDiskPreprocess();

            int n_query_item = query_item.n_vector_;
            int n_user = user_.n_vector_;

            std::vector<std::vector<UserRankElement>> query_heap_l(n_query_item, std::vector<UserRankElement>(topk));

            std::vector<double> queryIP_l(n_user);

            TimeRecord query_record;
            double tmp_inner_product_time = 0;
            double tmp_binary_search_time = 0;
            double tmp_read_disk_time = 0;
            query_record.reset();
            for (int qID = 0; qID < n_query_item; qID++) {
                //calculate distance
                double *query_item_vec = query_item.getVector(qID);
                inner_product_record_.reset();
                for (int userID = 0; userID < n_user; userID++) {
                    double *user_vec = user_.getVector(userID);
                    double queryIP = InnerProduct(query_item_vec, user_vec, vec_dim_);
                    queryIP_l[userID] = queryIP;
                }
                this->inner_product_time_ += inner_product_record_.get_elapsed_time_second();
                tmp_inner_product_time += inner_product_record_.get_elapsed_time_second();

                for (int userID = 0; userID < topk; userID++) {
                    int rank = tree_ins_.Retrieval(queryIP_l[userID], userID, read_disk_time_, binary_search_time_,
                                                   qID);
                    UserRankElement ele(userID, rank, queryIP_l[userID]);
                    query_heap_l[qID][userID] = ele;
                }

                std::make_heap(query_heap_l[qID].begin(), query_heap_l[qID].end(), std::less<UserRankElement>());

                for (int userID = topk; userID < n_user; userID++) {
                    UserRankElement min_heap_ele = query_heap_l[qID].front();
                    double single_read_disk_time, single_binary_search_time;
                    int rank = tree_ins_.Retrieval(queryIP_l[userID], userID, single_read_disk_time,
                                                   single_binary_search_time,
                                                   qID);
                    read_disk_time_ += single_read_disk_time;
                    binary_search_time_ += single_binary_search_time;
                    tmp_read_disk_time += single_read_disk_time;
                    tmp_binary_search_time += single_binary_search_time;

                    double queryIP = queryIP_l[userID];
                    UserRankElement element(userID, rank, queryIP);
                    if (min_heap_ele > element) {
                        std::pop_heap(query_heap_l[qID].begin(), query_heap_l[qID].end(), std::less<UserRankElement>());
                        query_heap_l[qID].pop_back();
                        query_heap_l[qID].push_back(element);
                        std::push_heap(query_heap_l[qID].begin(), query_heap_l[qID].end(),
                                       std::less<UserRankElement>());
                    }
                }

                std::sort(query_heap_l[qID].begin(), query_heap_l[qID].end(), std::less<UserRankElement>());

                if (qID % report_query_every_ == 0) {
                    spdlog::info("top-{} retrieval query number {}%, {} s/iter Mem: {} Mb", topk,
                                 qID / (0.01 * n_query_item),
                                 query_record.get_elapsed_time_second(), get_current_RSS() / 1000000);
                    query_record.reset();
                    printf("tmp: inner product %.3fs, binary search %.3fs, read disk %.3fs\n",
                           tmp_inner_product_time, tmp_binary_search_time, tmp_read_disk_time);
                    tmp_inner_product_time = 0;
                    tmp_binary_search_time = 0;
                    tmp_read_disk_time = 0;
                }
            }

            tree_ins_.in_stream_.close();
            return query_heap_l;
        }

    };

    const int write_every_ = 30000;
    const int report_batch_every_ = 5;

    /*
     * bruteforce index
     * shape: 1, type: int, n_data_item
     * shape: n_user * n_data_item, type: double, the distance pair for each user
     */

    Index &BuildIndex(VectorMatrix &data_item, VectorMatrix &user, const char *index_path) {
        const int n_data_item = data_item.n_vector_;
        std::vector<double> distance_cache(write_every_ * n_data_item);
        const int vec_dim = data_item.vec_dim_;
        const int n_batch = user.n_vector_ / write_every_;
        const int n_remain = user.n_vector_ % write_every_;
        user.vectorNormalize();

        const int node_size = 5;
        TreeIndex tree_ins(index_path, n_data_item, node_size);

        TimeRecord batch_report_record;
        batch_report_record.reset();
        for (int i = 0; i < n_batch; i++) {
#pragma omp parallel for default(none) shared(i, data_item, user, distance_cache) shared(write_every_, n_data_item, vec_dim)
            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                int userID = write_every_ * i + cacheID;
                for (int itemID = 0; itemID < n_data_item; itemID++) {
                    double ip = InnerProduct(data_item.getVector(itemID), user.getVector(userID), vec_dim);
                    distance_cache[cacheID * n_data_item + itemID] = ip;
                }
                std::sort(distance_cache.begin() + cacheID * n_data_item,
                          distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());
            }

            for (int cacheID = 0; cacheID < write_every_; cacheID++) {
                const double *distance_vecs = distance_cache.data() + cacheID * n_data_item;
                tree_ins.BuildWriteTree(distance_vecs);
            }

            if (i % report_batch_every_ == 0) {
                spdlog::info("preprocessed {}%, {} s/iter Mem: {} Mb", i / (0.01 * n_batch),
                             batch_report_record.get_elapsed_time_second(), get_current_RSS() / 1000000);
                batch_report_record.reset();
            }

        }

        if (n_remain != 0) {
            for (int cacheID = 0; cacheID < n_remain; cacheID++) {
                int userID = cacheID + write_every_ * n_batch;
                for (int itemID = 0; itemID < data_item.n_vector_; itemID++) {
                    double ip = InnerProduct(data_item.getRawData() + itemID * vec_dim,
                                             user.getRawData() + userID * vec_dim, vec_dim);
                    distance_cache[cacheID * data_item.n_vector_ + itemID] = ip;
                }

                std::sort(distance_cache.begin() + cacheID * n_data_item,
                          distance_cache.begin() + (cacheID + 1) * n_data_item, std::greater<double>());
            }

            for (int cacheID = 0; cacheID < n_remain; cacheID++) {
                const double *distance_vecs = distance_cache.data() + cacheID * n_data_item;
                tree_ins.BuildWriteTree(distance_vecs);
            }
        }
        tree_ins.out_stream_.close();

        static Index index(tree_ins, n_data_item, user);
        return index;
    }

}

#endif //REVERSE_KRANKS_BPLUSTREE_HPP
