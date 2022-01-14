import os

if __name__ == '__main__':
    # dataset_l = ['audio', 'movielens', 'netflix', 'yahoomusic']
    dataset_l = ['netflix']
    for ds in dataset_l:
        os.system('python3 reorder/svd.py %s' % ds)
        os.system('python3 reorder/measure_n_dims.py %s %s' % ('svd', ds))
        # os.system('python3 reorder/measure_n_dims.py %s %s' % ('origin', ds))
