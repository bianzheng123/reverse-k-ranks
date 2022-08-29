#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    const std::vector<int> data = {1, 2, 4, 5, 5, 6};
    for (int i = 0; i < 8; ++i) {
        // Search for first element x such that i ≤ x
        auto lower = std::lower_bound(data.begin(), data.end(), i,
                                      [](const int &info, int value) {
                                          return info <= value;
                                      });

        std::cout << i << " ≤ ";
        lower != data.end()
        ? std::cout << *lower << " at index " << std::distance(data.begin(), lower)
        : std::cout << "not found";
        std::cout << '\n';
    }

}