#include <algorithm>
#include <iostream>
#include <vector>

class PriceInfo {
public:
    double price;

    inline bool operator==(const PriceInfo &other) const {
        if (this == &other)
            return true;
        return price == other.price;
    };

    inline bool operator!=(const PriceInfo &other) const {
        if (this == &other)
            return false;
        return price != other.price;
    };

    inline bool operator<(const PriceInfo &other) const {
        return price < other.price;
    }

    inline bool operator<=(const PriceInfo &other) const {
        return price <= other.price;
    }

    inline bool operator>(const PriceInfo &other) const {
        return price > other.price;
    }

    inline bool operator>=(const PriceInfo &other) const {
        return price >= other.price;
    }
};

int main() {
//    const std::vector<int> data = {1, 2, 4, 5, 5, 6};
//    for (int i = 0; i < 8; ++i) {
//        // Search for first element x such that i ≤ x
//        auto lower = std::lower_bound(data.begin(), data.end(), i);
//
//        std::cout << i << " ≤ ";
//        lower != data.end()
//        ? std::cout << *lower << " at index " << std::distance(data.begin(), lower)
//        : std::cout << "[not found]";
//        std::cout << '\n';
//    }

//    std::vector<PriceInfo> prices = {{107.3},
//                                     {102.5},
//                                     {102.5},
//                                     {101.5},
//                                     {100.0}};
//    for (double to_find: {106.5, 110.2}) {
//        auto prc_info = std::lower_bound(prices.begin(), prices.end(), to_find,
//                                         [](const PriceInfo &info, double value) {
//                                             return info.price > value;
//                                         });
//
//        prc_info != prices.end()
//        ? std::cout << prc_info->price << " at index " << prc_info - prices.begin()
//        : std::cout << to_find << " not found";
//        std::cout << '\n';
//    }

    std::vector<PriceInfo> prices = {{107.3},
                                     {102.5},
                                     {101.5},
                                     {100.0}};
    for (double to_find: {106.1, 99.1}) {
        auto prc_info = std::lower_bound(prices.begin(), prices.begin() + 4, to_find,
                                         [](const PriceInfo &info, double value) {
                                             return info.price > value;
                                         });

        prc_info != prices.end()
        ? std::cout << prc_info->price << " at index " << prc_info - prices.begin()
        : std::cout << to_find << " not found";
        std::cout << '\n';
    }
    printf("%d\n", prices.end() - prices.begin());
}