import numpy as np
import pandas as pd
import os

if __name__ == '__main__':
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big']
    dataset_l = ['movielens-27m']
    topk_l = [100, 200, 300, 400]
    para_config_m = {'movielens-27m': 104, 'netflix': 33}
    for ds in dataset_l:
        for method_name in ['RankSample', 'QueryRankSample']:
            for topk in topk_l:
                basic_dir = 'data/sensitive_with_k/reason'
                fname = '{}-{}-top{}-n_sample_{}-userID.csv'.format(ds, method_name, topk,
                                                                    para_config_m[ds])
                df = pd.read_csv(os.path.join(basic_dir, fname))
                n_refine = np.average(df['n_user_candidate'])
                io_cost = np.average(df['io_cost'])
                print("{} {} top-{} n_refine: {}, io_cost: {}".format(ds, method_name, topk, n_refine, io_cost))
