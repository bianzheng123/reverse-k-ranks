import numpy as np
import pandas as pd
import os
import sys

sys.path.append('/home/bianzheng/Dataset/Recommendation/libpmf')  # 要用绝对路径
from python import libpmf


def matrix_factorization(df, n_dim):
    n_user = np.max(df['userId'])
    n_item = np.max(df['itemId'])
    mf = libpmf.train_coo(row_idx=df['userId'] - 1, col_idx=df['itemId'] - 1,
                          obs_val=df['rating'],
                          m=n_user,
                          n=n_item, param_str='-k %d -n 15' % n_dim)
    user_l = mf['W']
    item_l = mf['H']
    return user_l, item_l


def split_dataset(u2i_m, rating_df):
    # n_sample_user = 1000
    n_sample_user = 10
    n_user = np.max(rating_df['userId'])
    sample_u_idx_l = np.random.permutation(n_user)[:n_sample_user] + 1
    test_df = pd.DataFrame(columns=['userId', 'itemId', 'rating'])
    train_del_idx_l = []

    for u_idx in sample_u_idx_l:
        tmp_csv = u2i_m[u_idx]
        item_idx = np.argmax(tmp_csv['rating'])
        test_df.loc[tmp_csv.iloc[item_idx].name] = tmp_csv.iloc[item_idx].tolist()
        train_del_idx_l.append(tmp_csv.iloc[item_idx].name)
    train_df = rating_df.drop(train_del_idx_l, axis='index')
    test_df['userId'] = test_df['userId'].astype(np.int32)
    test_df['itemId'] = test_df['itemId'].astype(np.int32)
    return train_df, test_df


def run(filename, n_dim):
    topN = 10

    rating_df = pd.read_csv(filename)
    user2item_map = get_user2item_map(rating_df)
    n_user = np.max(rating_df['userId'])
    n_item = np.max(rating_df['itemId'])
    train_df, test_df = split_dataset(user2item_map, rating_df)
    user_l, item_l = matrix_factorization(train_df, n_dim)

    # print(test_df)
    for i in range(len(test_df)):
        uid = int(test_df.iloc[i]['userId']) - 1
        iid = int(test_df.iloc[i]['itemId']) - 1
        tmp_user = user_l[uid]

        item_idx_l = user2item_map[uid + 1]['itemId'].to_numpy() - 1
        # tmp_item_l = item_l[item_idx_l, :]

        # ip_l = np.array([np.dot(tmp_user, tmp_item_l[j]) for j in range(len(tmp_item_l))])
        ip_l = np.array([np.dot(tmp_user, item_l[j]) for j in range(len(item_l))])
        arg_l = np.argsort(-ip_l)  # 从大到小的排序序号
        new_ip_l = ip_l[arg_l]
        print(new_ip_l[:10])
        # print(new_ip_l)
        # print(arg_l)
        iid_idx = np.where(arg_l == iid)[0][0]
        print(uid, iid, iid_idx)


def get_user2item_map(ratings):
    n_user = np.max(ratings['userId'])
    n_item = np.max(ratings['itemId'])
    user2item_m = dict()
    for uid in range(1, n_user + 1, 1):
        rat_item_l = ratings[ratings['userId'] == uid]
        # print(uid, rat_item_l)
        user2item_m[uid] = rat_item_l
    return user2item_m


# hitting rate 关于dimensionality的曲线
if __name__ == '__main__':
    # filename_l = ['movielens-small', 'movielens-20m', 'movielens-25m', 'movielens-27m', 'netflix', 'amazon']
    filename_l = ['movielens-small']
    base_dir = '/home/bianzheng/Dataset/Recommendation'
    # n_dim_l = np.arange(10, 100, 10)
    n_dim_l = np.arange(100, 20, 10)
    n_dim_l = [150]
    for n_dim in n_dim_l:
        fn = filename_l[0]
        csv_filename = os.path.join(base_dir, '%s-ratings.csv' % fn)
        run(csv_filename, n_dim)
