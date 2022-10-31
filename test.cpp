#include <iostream>

#include "sdlp/sdlp.hpp"

using namespace std;
using namespace Eigen;

int main(int argc, char **argv) {
    int m = 2 * 7;
    Eigen::Matrix<double, 7, 1> x;        // decision variables
    Eigen::Matrix<double, 7, 1> c;        // objective coefficients
    Eigen::Matrix<double, -1, 7> A(m, 7); // constraint matrix
    Eigen::VectorXd b(m);                 // constraint bound

    c << 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0;
    A << 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
            -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0;
    b << 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0;

    double minobj = sdlp::linprog<7>(c, A, b, x);

    std::cout << "prob:\n"
              << std::endl;
    std::cout << "     min x1 + ... + x7," << std::endl;
    std::cout << "     s.t. x1 <=  6,  x2 <=  5, ..., x7 <= 0," << std::endl;
    std::cout << "          x1 >= -1,  x2 >= -2,  ..., x7 >= -7.\n"
              << std::endl;
    std::cout << "optimal sol: " << x.transpose() << std::endl;
    std::cout << "optimal obj: " << minobj << std::endl;

    Eigen::Matrix<double, 3, 1> cpy_x;

    double *ptr_c = new double[3];
    ptr_c[0] = 1;
    ptr_c[1] = 1;
    ptr_c[2] = 1;
//    Eigen::Matrix<double, 3, 1> cpy_c(ptr_c, 3);
//    Eigen::Matrix<double, 3, 1> cpy_c;
    Eigen::Map<Eigen::Matrix<double, 3, 1>> cpy_c(ptr_c, 3);
//    cpy_c[0] = 1;
//    cpy_c[1] = 2;
//    cpy_c[2] = 3;
    std::cout << cpy_c << std::endl;

    const int cpy_m = 3 * 2;
    double * ptr_A = new double[cpy_m * 3];
    ptr_A[0] = 1.0; ptr_A[1] = 0.0; ptr_A[2] = 0.0;
    ptr_A[3] = 0.0; ptr_A[4] = 1.0; ptr_A[5] = 0.0;
    ptr_A[6] = 0.0; ptr_A[7] = 0.0; ptr_A[8] = 1.0;

    ptr_A[9] = -1.0; ptr_A[10] = 0.0; ptr_A[11] = 0.0;
    ptr_A[12] = 0.0; ptr_A[13] = -1.0; ptr_A[14] = 0.0;
    ptr_A[15] = 0.0; ptr_A[16] = 0.0; ptr_A[17] = -1.0;

    using DMatrix3d = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;
    Eigen::Map<DMatrix3d> cpy_A(ptr_A, cpy_m, (int64_t) 3);
    std::cout << cpy_A << std::endl;

    double* ptr_b = new double[cpy_m];
    ptr_b[0] = 2.0; ptr_b[1] = 1.0; ptr_b[2] = 0.0;
    ptr_b[3] = 1.0; ptr_b[4] = 2.0; ptr_b[5] = 3.0;
    Eigen::Map<Eigen::VectorXd> cpy_b(ptr_b, cpy_m);
    std::cout << cpy_b << std::endl;

    minobj = sdlp::linprog<3>(cpy_c, cpy_A, cpy_b, cpy_x);

    std::cout << "optimal sol: " << cpy_x.transpose() << std::endl;
    std::cout << "optimal obj: " << minobj << std::endl;

    return 0;
}