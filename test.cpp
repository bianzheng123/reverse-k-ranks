//
// Created by BianZheng on 2021/12/27.
//

#include "struct/DistancePair.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;


int main(int argc, char **argv) {
    vector<ReverseMIPS::DistancePair> vecs;
    vecs.emplace_back(2, 1);
    vecs.emplace_back(4, 2);
    vecs.emplace_back(1, 3);
    sort(vecs.begin(), vecs.end(), greater<ReverseMIPS::DistancePair>());
    for(int i=0;i<vecs.size();i++){
        cout << vecs[i].ToString() << endl;
    }

    return 0;
}