#include <iostream>

#include "sdlp/sdlp.hpp"

using namespace std;
using namespace Eigen;

int main(int argc, char **argv) {
    std::vector<int> sample_rank_l = {1, 1, 3, 5, 7, 9, 11};
    const int size = sample_rank_l.size();
    const int rank = 1;
    int *rank_this_ptr = std::lower_bound(sample_rank_l.data(),
                                          sample_rank_l.data() + size,
                                          rank,
                                          [](const int &info, const int &value) {
                                              return info <= value;
                                          });
    const int queryID_rank_this = (int) (rank_this_ptr - sample_rank_l.data());

//    const int rank_next = rank + 1;
    int *rank_next_ptr = std::lower_bound(sample_rank_l.data(),
                                          sample_rank_l.data() + size,
                                          rank,
                                          [](const int &info, const int &value) {
                                              return info < value;
                                          });
    const int queryID_rank_next = (int) (rank_next_ptr - sample_rank_l.data());

    printf("queryID <= %d, < %d\n", queryID_rank_this, queryID_rank_next);


    return 0;
}