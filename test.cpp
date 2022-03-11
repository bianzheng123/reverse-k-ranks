//
// Created by BianZheng on 2022/3/10.
//

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>

using namespace std;

void print_arr(const std::vector<int> &arr) {
    for (const int i: arr) {
        printf("%d ", i);
    }
    printf("\n");
}

int main(int argc, char **argv) {

    std::vector<int> vec = {10, 4, 5, 2, 1, 19, 45};
    make_heap(vec.begin(), vec.end(), std::less());
    print_arr(vec);

    std::pop_heap(vec.begin(), vec.end(),
                  std::less());
    print_arr(vec);

    std::push_heap(vec.begin(), vec.end(),
                  std::less());
    print_arr(vec);


    std::vector<int> vec2 = {10, 4, 5, 2, 1, 19, 45};
    std::swap(vec2[1], vec2[2]);
    print_arr(vec2);

    printf("%d \n", vec2.front());

    return 0;
}