#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>

int main()
{
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::shuffle(v.begin(), v.end(), std::default_random_engine(0));

    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";
}