#include <algorithm>
#include <iostream>
#include <vector>

struct PriceInfo {
    int lb, ub;
};

int main() {
//    std::vector<PriceInfo> prices1 = {{3, 3},
//                                      {2, 3},
//                                      {2, 3},
//                                      {2, 2},
//                                      {1, 2},
//                                      {1, 1},
//                                      {1, 1}
//    };

//    std::vector<PriceInfo> prices = {{1, 1},
//                                     {1, 1},
//                                     {1, 2},
//                                     {1, 3},
//                                     {2, 2},
//                                     {2, 3},
//                                     {3, 3}};

    std::vector<PriceInfo> prices2 = {{1, 1},
                                      {1, 1},
                                      {1, 2},
                                      {2, 2},
                                      {1, 3},
                                      {2, 3},
                                      {3, 3}
    };

    printf("first\n");
    for (int to_find: {1, 2, 3}) {
        auto prc_info = std::lower_bound(prices2.begin(), prices2.end(), to_find,
                                         [](const PriceInfo &info, int value) {
                                             return info.ub < value;
                                         });

        prc_info != prices2.end()
        ? std::cout << prc_info->ub << " at index " << prc_info - prices2.begin() << " "
                    << (int) (prc_info == prices2.end())
        : std::cout << to_find << " not found";
        std::cout << '\n';
    }
//    printf("second\n");
//    for (int to_find: {1, 2, 3}) {
//        auto prc_info = std::lower_bound(prices.begin(), prices.end(), to_find,
//                                         [](const PriceInfo &info, int value) {
//                                             return info.lb <= value;
//                                         });
//
//        prc_info != prices.end()
//        ? std::cout << prc_info->lb << " at index " << prc_info - prices.begin()
//        : std::cout << to_find << " not found";
//        std::cout << '\n';
//    }
}