//
// Created by BianZheng on 2022/11/11.
//

#ifndef REVERSE_K_RANKS_BASELINEARREGRESSION_HPP
#define REVERSE_K_RANKS_BASELINEARREGRESSION_HPP
namespace ReverseMIPS {
    class BaseLinearRegression {
    public:

        inline BaseLinearRegression() {};

        virtual void StartPreprocess(const int *sampleIP_l, const int &n_sample_rank) = 0;

        virtual void LoopPreprocess(const double *sampleIP_l, const int &userID) = 0;

        virtual void FinishPreprocess() = 0;

        virtual void SaveIndex(const char *index_basic_dir, const char *dataset_name, const size_t &n_sample_query,
                               const size_t &sample_topk) = 0;

    };
}
#endif //REVERSE_K_RANKS_BASELINEARREGRESSION_HPP
