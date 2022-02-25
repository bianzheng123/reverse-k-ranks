#include <spdlog/spdlog.h>

struct A{
public:
    double dist;
    int id;
    inline A(double dist, int id){
        this->dist = dist;
        this->id = id;
    }
};

class B{
public:
    double dist;
    int id;

    inline B(double dist, int id){
        this->dist = dist;
        this->id = id;
    }
};

int main(int argc, char **argv) {
    printf("%d %d\n", sizeof(A), sizeof(B));
    return 0;
}