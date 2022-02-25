#include <algorithm>
#include <functional>
#include <iostream>
#include <string_view>
#include <vector>

void print(std::string_view text, std::vector<int> const &v = {}) {
    std::cout << text << ": ";
    for (const auto &e: v) std::cout << e << ' ';
    std::cout << '\n';
}

int main(int argc, char **argv) {



    std::vector<int> v{3, 2, 4, 1, 5, 9};
    v.assign(v.size(), 1);

    print("initially, v", v);

    return 0;
}