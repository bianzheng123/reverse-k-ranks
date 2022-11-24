#ifndef MATRIX_H
#define MATRIX_H

#include "../util/Base.h"
#include "VectorElement.h"

#include "struct/VectorMatrix.hpp"

class Matrix {

public:
    double *rawData;
    size_t rowNum;
    size_t colNum;

    inline Matrix() {
        this->rawData = NULL;
        this->rowNum = 0;
        this->colNum = 0;
    }

    Matrix(const Matrix &) = delete;

    Matrix &operator=(const Matrix &) = delete;

    Matrix(Matrix &&x)
            : rawData(x.rawData), rowNum(x.rowNum), colNum(x.colNum) {
        x.rawData = nullptr;
    }

    Matrix &operator=(Matrix rhs) noexcept {
        using std::swap;
        swap(this->rawData, rhs.rawData);
        swap(this->rowNum, rhs.rowNum);
        swap(this->colNum, rhs.colNum);
        return *this;
    }

    inline ~Matrix() {
        if (rawData) {
            delete[] rawData;
        }
    }

    inline double *getRowPtr(const int rowIndex) const {
        return &rawData[rowIndex * colNum];
    }

    inline double *getRowPtr(const int64_t rowIndex) const {
        return &rawData[rowIndex * colNum];
    }

    inline void init(double *rawData, const int rowNum, const int colNum) {
        this->rowNum = rowNum;
        this->colNum = colNum;
        this->rawData = rawData;
    }

    inline void init(const int rowNum, const int colNum) {
        this->rowNum = rowNum;
        this->colNum = colNum;
        this->rawData = new double[rowNum * colNum];
    }

    inline void init(const ReverseMIPS::VectorMatrix &vm) {
        this->rowNum = vm.n_vector_;
        this->colNum = vm.vec_dim_;
        this->rawData = new double[rowNum * colNum];
        uint64_t n_vector = rowNum;
        uint64_t vec_dim = colNum;
        std::memcpy(this->rawData, vm.getRawData(), n_vector * vec_dim * sizeof(double));
    }

    void TransformVectorMatrix() {}

    void readData(string dataFilePath) {
        vector<string> lines;
        string line;

        ifstream fin(dataFilePath.c_str());

        int rowNum = 0;
        int colNum = 0;

        while (getline(fin, line)) {
            if (line.length() == 0) {
                continue;
            }
            lines.push_back(line);
        }

        if (lines.size() == 0) {
            return;
        }

        fin.close();

        stringstream test(lines[0]);
        double tempValue;
        while (test >> tempValue) {
            if (test.peek() == ',') {
                test.ignore();
                colNum++;
            }
        }
        colNum++;
        rowNum = lines.size();

        this->rowNum = rowNum;
        this->colNum = colNum;
        this->rawData = new double[rowNum * colNum];
        int colIndex = 0;
        printf("rowNum %d, colNum %d\n", rowNum, colNum);

        for (int rowIndex = 0; rowIndex < lines.size(); rowIndex++) {
            stringstream ss(lines[rowIndex]);
            colIndex = 0;
            while (ss >> this->rawData[rowIndex * colNum + colIndex]) {
                if (ss.peek() == ',') {
                    ss.ignore();
                    colIndex++;
                }
            }
        }
    }
};

#endif //MATRIX_H