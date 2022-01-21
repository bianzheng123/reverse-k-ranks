//
// Created by BianZheng on 2022/1/20.
//

#ifndef REVERSE_KRANKS_KMEANS_HPP
#define REVERSE_KRANKS_KMEANS_HPP

#include <fstream>
#include "struct/Cluster.hpp"

namespace ReverseMIPS {

    class KMeans {
    private:
        int K, iters, dimensions, total_points;
        std::vector<Cluster> clusters;

        void clearClusters() {
            for (int i = 0; i < K; i++) {
                clusters[i].removeAllPoints();
            }
        }

        int getNearestClusterId(Point point) {
            double sum = 0.0, min_dist;
            int NearestClusterId;
            if (dimensions == 1) {
                min_dist = abs(clusters[0].getCentroidByPos(0) - point.getVal(0));
            } else {
                for (int i = 0; i < dimensions; i++) {
                    sum += pow(clusters[0].getCentroidByPos(i) - point.getVal(i), 2.0);
                    // sum += abs(clusters[0].getCentroidByPos(i) - point.getVal(i));
                }
                min_dist = sqrt(sum);
            }
            NearestClusterId = clusters[0].getId();

            for (int i = 1; i < K; i++) {
                double dist;
                sum = 0.0;

                if (dimensions == 1) {
                    dist = abs(clusters[i].getCentroidByPos(0) - point.getVal(0));
                } else {
                    for (int j = 0; j < dimensions; j++) {
                        sum += pow(clusters[i].getCentroidByPos(j) - point.getVal(j), 2.0);
                        // sum += abs(clusters[i].getCentroidByPos(j) - point.getVal(j));
                    }

                    dist = sqrt(sum);
                    // dist = sum;
                }
                if (dist < min_dist) {
                    min_dist = dist;
                    NearestClusterId = clusters[i].getId();
                }
            }

            return NearestClusterId;
        }

    public:
        KMeans(int K, int iterations) {
            this->K = K;
            this->iters = iterations;
        }

        std::vector<int> run(std::vector<Point> &all_points) {
            total_points = all_points.size();
            dimensions = all_points[0].getDimensions();

            // Initializing Clusters
            std::vector<int> used_pointIds;

            for (int i = 1; i <= K; i++) {
                while (true) {
                    int index = rand() % total_points;

                    if (find(used_pointIds.begin(), used_pointIds.end(), index) ==
                        used_pointIds.end()) {
                        used_pointIds.push_back(index);
                        all_points[index].setCluster(i);
                        Cluster cluster(i, all_points[index]);
                        clusters.push_back(cluster);
                        break;
                    }
                }
            }
            std::cout << "Running K-Means Clustering.." << std::endl;

            int iter = 1;
            const int report_every = 5;
            while (true) {
                if (iter % report_every == 0) {
                    std::cout << "Iter - " << iter << "/" << iters << std::endl;
                }
                bool done = true;

                // Add all points to their nearest cluster
#pragma omp parallel for reduction(&&: done) num_threads(16) shared(all_points) default(none)
                for (int i = 0; i < total_points; i++) {
                    int currentClusterId = all_points[i].getCluster();
                    int nearestClusterId = getNearestClusterId(all_points[i]);

                    if (currentClusterId != nearestClusterId) {
                        all_points[i].setCluster(nearestClusterId);
                        done = false;
                    }
                }

                // clear all existing clusters
                clearClusters();

                // reassign points to their new clusters
                for (int i = 0; i < total_points; i++) {
                    // cluster index is ID-1
                    clusters[all_points[i].getCluster() - 1].addPoint(all_points[i]);
                }

                // Recalculating the center of each cluster
                for (int i = 0; i < K; i++) {
                    int ClusterSize = clusters[i].getSize();

                    for (int j = 0; j < dimensions; j++) {
                        double sum = 0.0;
                        if (ClusterSize > 0) {
#pragma omp parallel for reduction(+: sum) num_threads(16) shared(clusters, ClusterSize, i, j) default(none)
                            for (int p = 0; p < ClusterSize; p++) {
                                sum += clusters[i].getPoint(p).getVal(j);
                            }
                            clusters[i].setCentroidByPos(j, sum / ClusterSize);
                        }
                    }
                }

                if (done || iter >= iters) {
                    std::cout << "Clustering completed in iteration : " << iter << std::endl
                              << std::endl;
                    break;
                }
                iter++;
            }

            std::vector<int> label_l(total_points);
            for (int i = 0; i < total_points; i++) {
                label_l[i] = all_points[i].getCluster() - 1;
            }

            //id顺序增加
//            std::vector<int> id_l(total_points);
//            for (int i = 0; i < total_points; i++) {
//                id_l[i] = all_points[i].getID();
//            }
//            for (int i = 0; i < total_points; i++) {
//                std::cout << id_l[i] << " " << label_l[i] << std::endl;
//            }

            return label_l;
        }
    };

    std::vector<Point> vectorMatrix2Point(VectorMatrix &vm) {
        int n_point = vm.n_vector_;
        int n_dim = vm.vec_dim_;
        std::vector<Point> point_l;
        for (int i = 0; i < n_point; i++) {
            point_l.emplace_back(i, n_dim, vm.rawData_ + i * n_dim);
        }
        return point_l;
    }

    std::vector<int> BuildKMeans(VectorMatrix &user, int n_merge_user) {
        std::vector<Point> point_l = vectorMatrix2Point(user);

        const int n_iter = 100;
        KMeans kmeans(n_merge_user, n_iter);
        std::vector<int> label_l = kmeans.run(point_l);
        return label_l;
    }

}
#endif //REVERSE_KRANKS_KMEANS_HPP
