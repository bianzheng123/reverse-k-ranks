#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <spdlog/spdlog.h>

int main() {
    const char *dataset_name = "goodreads";
    const int n_sample_item = 5000;
    const int sample_topk = 30;

    std::vector<int> topk_rank_l(n_sample_item);

    char resPath[256];
    std::sprintf(resPath,
                 "../index/query_distribution/%s-below-topk-n_sample_query_%d-sample_topk_%d.index",
                 dataset_name, n_sample_item, sample_topk);

    std::ifstream out_stream = std::ifstream(resPath, std::ios::binary);
    if (!out_stream) {
        spdlog::error("error in write result");
        exit(-1);
    }

    out_stream.read((char *) topk_rank_l.data(),
                    (std::streamsize) (sizeof(int) * n_sample_item));

    out_stream.close();

    int count0 = 0;
    for (int queryID = 0; queryID < n_sample_item; queryID++) {
        if (topk_rank_l[queryID] == 0) {
            count0++;
        }
    }
    printf("# 0 rank: %d\n", count0);

}