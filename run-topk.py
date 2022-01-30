import os

if __name__ == '__main__':
    dataset_l = ['movielens-27m', 'netflix', 'yahoomusic', 'yelp']
    for ds in dataset_l:
        os.system('cd build && ./bfdi %s' % ds)
