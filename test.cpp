#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    const int n_data_item = 53889;
    const int n_user = 283228;
    const int n_read = n_data_item * n_user * sizeof(double);
    printf("n_read %d\n", n_read);
}