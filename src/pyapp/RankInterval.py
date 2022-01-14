import numpy as np
import os
from util import vecs_io
from sklearn.cluster import KMeans

n_merge_user = 0


def delete_dir_if_exist(dire):
    if os.path.isdir(dire):
        command = 'rm -rf %s' % dire
        print(command)
        os.system(command)


def merge_user(user_l, n_merge_user):
    kmeans = KMeans(n_clusters=n_merge_user, random_state=0).fit(user_l)
    return kmeans.labels_


def run(dataset):
    os.system("cd build && cmake ..")
    basic_dir = '/home/bianzheng/Dataset/MIPS/Reverse-kRanks'
    user, dim = vecs_io.fvecs_read('%s/%s/%s_user.fvecs' % (basic_dir, dataset, dataset))
    n_merge_user = min(int(len(user) / 10), 1000)
    user_idx_l = merge_user(user, n_merge_user)

    index_dir = '/home/bianzheng/Reverse-kRanks/index/RankInterval-%s' % dataset
    delete_dir_if_exist(index_dir)
    os.mkdir(index_dir)
    index_merge_user_dir = '%s/user_idx.txt' % index_dir

    with open(index_merge_user_dir, 'w') as f:
        f.write('%d,%d\n' % (len(user), n_merge_user))
        for ele in user_idx_l:
            f.write("%d\n" % ele)

    os.system("cd build && make && ./rkitv %s 10" % dataset)


if __name__ == "__main__":
    # dataset_l = ['fake', 'movielens-small']
    dataset_l = ['fake']
    for ds in dataset_l:
        run(ds)
