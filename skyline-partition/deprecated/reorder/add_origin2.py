import numpy as np
import os


def delete_dir_if_exist(dire):
    if os.path.isdir(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


if __name__ == '__main__':
    dataset_name_l = ['audio', 'movielens', 'netflix', 'yahoomusic']
    for ds in dataset_name_l:
        delete_dir_if_exist('data/%s' % ds)
        os.system('mkdir -p data/%s' % ds)
        os.system('cp /home/bianzheng/Dataset/NIPS/%s/%s_base.fvecs data/%s/origin.fvecs' % (ds, ds, ds))
        os.system('cp /home/bianzheng/Dataset/NIPS/%s/%s_query.fvecs data/%s/origin_query.fvecs' % (ds, ds, ds))
