#include <algorithm>
#include <iostream>
#include <vector>

struct PriceInfo {
    double price;
};

int main() {

    std::vector<PriceInfo> prices = {{107.3},
                                     {102.5},
                                     {102.5},
                                     {101.5},
                                     {100.0}};

    for (double to_find: {102.5, 110.2, 101.0}) {
        auto prc_info = std::lower_bound(prices.begin(), prices.end(), to_find,
                                         [](const PriceInfo &info, double value) {
                                             return info.price > value;
                                         });

        printf("lower bound price %.3f at index %d\n", to_find, prc_info - prices.begin());

        prc_info = std::upper_bound(prices.begin(), prices.end(), to_find,
                                         [](double value, const PriceInfo &info) {
                                             return value > info.price;
                                         });

        printf("upper bound price %.3f at index %d\n", to_find, prc_info - prices.begin());

    }

}