#include <stdio.h>
#include <vector>
#include <algorithm>

int main() {
    const std::vector<int> data = {1, 2, 4, 5, 5, 6};

    auto ptr1 = std::lower_bound(data.begin(), data.end(), 5,
                                  [](const int &info, const int& value) {
                                      return info < value;
                                  });

    auto ptr2 = std::lower_bound(data.begin(), data.end(), 1,
                                     [](const int &info, const int& value) {
                                         return info <= value;
                                     });
    printf("%d %d\n", ptr1 - data.begin(), ptr2 - data.begin());

    return 0;
}