//
// Created by BianZheng on 2022/2/25.
//

#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int main(int argc, char **argv) {
    std::vector<std::vector<int>> vecs_l;
    std::vector<int> vecs = {1, 2, 3, 4, 5, 6};
    vecs_l.push_back(vecs);

    std::vector<int> vecs1 = {1, 2, 3};
    vecs_l[0] = vecs1;
    printf("%d, %d\n", (int) vecs.size(), (int) vecs_l[0].size());
    return 0;
}