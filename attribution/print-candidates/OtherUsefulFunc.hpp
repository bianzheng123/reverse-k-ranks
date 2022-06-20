//
// Created by BianZheng on 2022/6/20.
//

#ifndef REVERSE_KRANKS_OTHERUSEFULFUNC_HPP
#define REVERSE_KRANKS_OTHERUSEFULFUNC_HPP

namespace ReverseMIPS {
    void
    WriteRankResult(const std::vector<std::vector<UserRankElement>> &result, const char *dataset_name,
                    const char *method_name, const char *other_name) {
        int n_query_item = (int) result.size();
        int topk = (int) result[0].size();

        char resPath[256];
        std::sprintf(resPath, "../../result/rank/%s-%s-top%d-%s-userID.csv", dataset_name, method_name, topk,
                     other_name);
        std::ofstream file(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].userID_ << ",";
            }
            file << result[i][topk - 1].userID_ << std::endl;
        }
        file.close();

        std::sprintf(resPath, "../../result/rank/%s-%s-top%d-%s-rank.csv", dataset_name, method_name, topk, other_name);
        file.open(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].rank_ << ",";
            }
            file << result[i][topk - 1].rank_ << std::endl;
        }
        file.close();

        std::sprintf(resPath, "../../result/rank/%s-%s-top%d-%s-IP.csv", dataset_name, method_name, topk, other_name);
        file.open(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int i = 0; i < n_query_item; i++) {
            for (int j = 0; j < topk - 1; j++) {
                file << result[i][j].queryIP_ << ",";
            }
            file << result[i][topk - 1].queryIP_ << std::endl;
        }
        file.close();
    }

    class ItemCandidates {
    public:
        int userID_;
        std::vector<int> itemID_l_;

        inline ItemCandidates() = default;

        inline ItemCandidates(const std::vector<int> &itemID_l, const int &userID) {
            this->itemID_l_ = itemID_l;
            this->userID_ = userID;
        }
    };

    void
    WriteCandidateResult(const std::vector<std::vector<ItemCandidates>> &result, const int &topk,
                         const char *dataset_name, const char *method_name, const char *other_name) {
        int n_query_item = (int) result.size();

        char resPath[256];
        std::sprintf(resPath, "../../result/rank/%s-%s-top%d-%s.txt", dataset_name, method_name, topk, other_name);
        std::ofstream file(resPath);
        if (!file) {
            spdlog::error("error in write result");
        }

        for (int qID = 0; qID < n_query_item; qID++) {
            file << qID << ":" << std::endl;
            for (const ItemCandidates &ucandID: result[qID]) {
                file << ucandID.userID_ << ":";
                int item_cand_size = int(ucandID.itemID_l_.size());
                if (item_cand_size == 0) {
                    file << "size0" << std::endl;
                } else {
                    for (int item_candID = 0; item_candID < item_cand_size - 1; item_candID++) {
                        file << ucandID.itemID_l_[item_candID] << ",";
                    }
                    file << ucandID.itemID_l_[ucandID.itemID_l_.size() - 1] << std::endl;
                }

            }
        }
        file.close();
    }
}
#endif //REVERSE_KRANKS_OTHERUSEFULFUNC_HPP
