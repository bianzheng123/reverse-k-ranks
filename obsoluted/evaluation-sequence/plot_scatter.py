import numpy as np
import matplotlib.pyplot as plt

if __name__ == '__main__':
    dataset_l = ['movielens-small']
    for ds in dataset_l:
        eval_seq = np.loadtxt('../../result/attribution/CauchyPercentile/{}-CauchyPercentile-evaluation-sequence.txt'.format(ds))
        n_item = len(eval_seq)
        rand_seq = np.random.permutation(n_item)
        index_l = np.loadtxt('../../result/attribution/CauchyPercentile/{}-CauchyPercentile-all-index.csv'.format(ds), delimiter=',')
        n_query = len(index_l)
        eval_point_l = []
        # first dimension: real rank, second dimension: predict rank
        for i in range(n_item):
            eval_idx = eval_seq[i]
            idx_l = np.argwhere(index_l == eval_idx)
            assert len(idx_l) == n_query
            rank_l = np.array([_[1] for _ in idx_l]) + 1
            rank_l = rank_l[:, np.newaxis]
            point_l = np.ones((n_query, 1), dtype=np.int32) * i + 1
            point_l = np.concatenate((rank_l, point_l), axis=1)
            if i == 0:
                eval_point_l = point_l
            else:
                eval_point_l = np.append(eval_point_l, point_l, axis=0)
        print("eval point finish")

        rand_point_l = []
        for i in range(n_item):
            eval_idx = rand_seq[i]
            idx_l = np.argwhere(index_l == eval_idx)
            assert len(idx_l) == n_query
            rank_l = np.array([_[1] for _ in idx_l]) + 1
            rank_l = rank_l[:, np.newaxis]
            point_l = np.ones((n_query, 1), dtype=np.int32) * i + 1
            point_l = np.concatenate((rank_l, point_l), axis=1)
            if i == 0:
                rand_point_l = point_l
            else:
                rand_point_l = np.append(rand_point_l, point_l, axis=0)

        eval_rho = np.corrcoef(eval_point_l[:, 0], eval_point_l[:, 1])
        rand_rho = np.corrcoef(rand_point_l[:, 0], rand_point_l[:, 1])
        print("eval rho", eval_rho)
        print("rand rho", rand_rho)