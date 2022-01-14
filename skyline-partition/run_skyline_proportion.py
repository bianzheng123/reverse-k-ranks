import os

if __name__ == '__main__':
    dataset_l = ['netflix', 'movielens-20m']
    prefix_l = ['item', 'item_svd', 'item_fexipro_svd']
    n_eval_l = [-1, 20]
    for ds in dataset_l:
        for prefix in prefix_l:
            for n_eval in n_eval_l:
                os.system("cd build && ./skyline %s %s %d" % (ds, prefix, n_eval))
