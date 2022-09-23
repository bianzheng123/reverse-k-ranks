#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <spdlog/spdlog.h>
#include "util/TimeMemory.hpp"

int main() {
    ReverseMIPS::TimeRecord record;
    record.reset();
    for(int i=0;i<10000;i++){
        system("# sync; echo 3 > /proc/sys/vm/drop_caches");
    }
    const double time = record.get_elapsed_time_second();
    printf("used time %.3f\n", time);

}