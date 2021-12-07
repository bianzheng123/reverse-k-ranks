#include <iostream>
#include <vector>
#include <queue>
#include "src/struct/RankElement.hpp"

using namespace ReverseMIPS;
using namespace std;


int main(int argc, char **argv) {
    priority_queue<RankElement, vector<RankElement>, less<RankElement> > q;
    q.push(RankElement(10, 6));
    q.push(RankElement(10, 10));
    q.push(RankElement(10, 1));
    q.push(RankElement(10, 2));
    while (!q.empty()) {
        RankElement tmp = q.top();
        printf("%d ", tmp.rank_);
        q.pop();
    }
    printf("\n");
    return 0;
}