import numpy as np
import pandas as pd
import os

if __name__ == '__main__':
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big']
    # dataset_l = ['movielens-27m']
    dataset_l = ['netflix']
    para_config_m = {'movielens-27m': 104, 'netflix': 33}
    for ds in dataset_l:
        # for method_name in ['RankSample', 'QueryRankSample']:
        for method_name in ['RSTopTIP', 'QRSTopTIP']:
            basic_dir = 'data/single_query_performance'
            fname = '{}-{}-top10-n_sample_{}-index_size_gb_256-userID.csv'.format(ds, method_name, para_config_m[ds])
            df = pd.read_csv(os.path.join(basic_dir, fname))
            n_refine = np.average(df['n_user_candidate'])
            io_cost = np.average(df['io_cost'])
            print("{} {} n_refine: {}, io_cost: {}".format(ds, method_name, n_refine, io_cost))
