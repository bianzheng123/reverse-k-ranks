//
// Created by BianZheng on 2022/1/20.
//

#ifndef REVERSE_KRANKS_CLUSTER_HPP
#define REVERSE_KRANKS_CLUSTER_HPP

#include <vector>

namespace ReverseMIPS {
    class Point {
    private:
        int pointId, clusterId;
        int dimensions;
        double *values;

    public:
        Point(int id, int dimension, double *values_ptr) {
            this->pointId = id;
            this->values = values_ptr;
            this->dimensions = dimension;
            this->clusterId = 0; // Initially not assigned to any cluster
        }

        [[nodiscard]] int getDimensions() const { return dimensions; }

        [[nodiscard]] int getCluster() const { return clusterId; }

        [[nodiscard]] int getID() const { return pointId; }

        void setCluster(int val) { clusterId = val; }

        [[nodiscard]] double getVal(int pos) const { return values[pos]; }
    };

    class Cluster {
    private:
        int clusterId;
        std::vector<double> centroid;
        std::vector<Point> points;

    public:
        Cluster(int clusterId, Point centroid) {
            this->clusterId = clusterId;
            for (int i = 0; i < centroid.getDimensions(); i++) {
                this->centroid.push_back(centroid.getVal(i));
            }
            this->addPoint(centroid);
        }

        void addPoint(Point p) {
            p.setCluster(this->clusterId);
            points.push_back(p);
        }

        bool removePoint(int pointId) {
            int size = points.size();

            for (int i = 0; i < size; i++) {
                if (points[i].getID() == pointId) {
                    points.erase(points.begin() + i);
                    return true;
                }
            }
            return false;
        }

        void removeAllPoints() { points.clear(); }

        int getId() { return clusterId; }

        Point getPoint(int pos) { return points[pos]; }

        int getSize() { return points.size(); }

        double getCentroidByPos(int pos) { return centroid[pos]; }

        void setCentroidByPos(int pos, double val) { this->centroid[pos] = val; }
    };
}
#endif //REVERSE_KRANKS_CLUSTER_HPP
