//
// Created by BianZheng on 2022/2/25.
//

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include <spdlog/spdlog.h>

using namespace std;

int main(int argc, char **argv) {
    std::vector<int> haystack{1, 4, 3, 10, 5, 9};
    std::sort(haystack.begin(), haystack.end(), std::less());
    for(const int num: haystack){
        printf("%d ", num);
    }
    printf("\n");
    return 0;
}