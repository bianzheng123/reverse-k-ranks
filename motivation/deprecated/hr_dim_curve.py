import numpy as np
import pandas as pd
import os
import sys
import matplotlib.pyplot as plt

sys.path.append('/home/bianzheng/Dataset/Recommendation/libpmf')
from python import libpmf

n_recall = 20
n_sample_user = 1000
n_rating_thres = 500


def draw_curve(x, y, name):
    # 第一个是横坐标的值，第二个是纵坐标的值
    # plt.figure(num=3, figsize=(8, 5))
    # marker
    # o 圆圈, v 倒三角, ^ 正三角, < 左三角, > 右三角, 8 大圆圈, s 正方形, p 圆圈, * 星号, h 菱形, H 六面体, D 大菱形, d 瘦的菱形, P 加号, X 乘号
    # 紫色#b9529f 蓝色#3953a4 红色#ed2024 #231f20 深绿色#098140 浅绿色#7f8133 #0084ff
    # solid dotted

    marker_l = ['H', 'D', 'P', '>', '*', 'X', 's', '<', '^', 'p', 'v']
    color_l = ['#b9529f', '#3953a4', '#ed2024', '#098140', '#231f20', '#7f8133', '#0084ff']
    plt.plot(x, y, marker=marker_l[0], linestyle='solid',
             color=color_l[0])
    # plt.xscale('log')
    # plt.xlim(1, 500000)

    # 使用legend绘制多条曲线
    # plt.title('graph kmeans vs knn')
    plt.legend(loc='upper left', title="legend")
    plt.title("dimensionality vs hitting rate curve")

    plt.xlabel("dimensionality")
    plt.ylabel("hitting rate")
    plt.grid(True, linestyle='-.')
    # plt.xticks([0, 0.1, 0.2, 0.3, 0.4])
    # plt.yticks([0.75, 0.8, 0.85])
    plt.savefig('result/hr_dim_curve/%s.jpg' % (name))
    plt.close()


def get_train_set(filename, fn):
    rating_df = pd.read_csv(filename)
    rating_df['userId'] = rating_df['userId'] - 1
    rating_df['itemId'] = rating_df['itemId'] - 1
    user2item_id_m, test_total_global_idx_l, user2max_item_id_m, total_n_test_l = split_dataset(rating_df)
    train_set = rating_df.drop(test_total_global_idx_l, inplace=False)
    # print(total_n_test_l)
    np.savetxt('result/hr_dim_curve/%s.txt' % fn, total_n_test_l, fmt="%d")
    return user2item_id_m, user2max_item_id_m, train_set
    # return user2item_id_m, user2max_item_id_m, rating_df


def calc_hitting_rate(trainset_item, dim: int) -> float:
    user2item_id_m, user2max_item_id_m, train_set = trainset_item
    # print(test_set)
    # print(train_set)
    user_l, item_l = matrix_factorization(train_set, dim)
    match_count = 0
    for uid in user2item_id_m.keys():
        user_vecs = user_l[uid, :]
        tmp_item_id_l = np.array(user2item_id_m[uid])
        item_vecs_l = item_l[tmp_item_id_l, :]
        max_item_id = user2max_item_id_m[uid]
        max_item_idx = np.argwhere(tmp_item_id_l == max_item_id)[0][0]
        # print(len(tmp_item_id_l), max_item_idx)
        ip_l = np.array([np.dot(user_vecs, item_vecs) for item_vecs in item_vecs_l])
        arg_idx_l = np.array(np.argsort(-ip_l))
        # print("max item ip", ip_l[-1])
        # print(ip_l[arg_idx_l][:10])
        candidate_l = arg_idx_l[:n_recall]
        # print(candidate_l)
        # print(max_item_idx)
        match_count += 1 if max_item_idx in candidate_l else 0
    print("match count %d, total sample %d" % (match_count, len(user2max_item_id_m.keys())))
    # print(len(user2max_item_id_m.keys()))
    return match_count / len(user2max_item_id_m.keys())


def split_dataset(rating_df: pd.DataFrame):
    # 先把数据集选出来, 选出评分数量大于500的user, 然后从这些user中随机抽1000个
    # 对于筛选出的user, 先挑出一个最大的rating, 作为test set, 然后从不大于最大的rating的数量中随机挑选rating, 作为评估的结果
    group_ins = rating_df.groupby(['userId'])
    sample_uid_l = get_sample_user_id(group_ins, n_sample_user, n_rating_thres)
    # print(sample_uid_l)

    user2item_id_m = {}
    test_total_global_idx_l = []
    user2max_item_id_m = {}
    total_n_test_l = []

    # 对于user, 选择测试用的item
    print("same uid len ", len(sample_uid_l))
    for uid in sample_uid_l:
        user_df = group_ins.get_group(uid)
        max_idx = np.argmax(user_df['rating'])
        max_rating = user_df.iloc[max_idx]['rating']
        max_item_id = int(user_df.iloc[max_idx]['itemId'])
        max_global_idx = user_df.iloc[max_idx].name

        less_rating_df = user_df.where(user_df['rating'] < max_rating)
        less_rating_df.dropna(inplace=True)
        test_small_rating_size = len(less_rating_df) // 2
        test_idx_l = np.random.permutation(len(less_rating_df))[:test_small_rating_size]
        tmp_test_df = less_rating_df.iloc[test_idx_l]
        total_n_test_l.append(len(test_idx_l) + 1)

        test_global_idx_l = tmp_test_df.index.tolist()
        test_global_idx_l.append(max_global_idx)
        test_total_global_idx_l.extend(test_global_idx_l)
        # print(tmp_test_df)
        tmp_item_id_l = tmp_test_df['itemId'].astype(np.int32).tolist()
        tmp_item_id_l.append(max_item_id)
        user2item_id_m[uid] = tmp_item_id_l
        user2max_item_id_m[uid] = max_item_id
        # print(max_idx, max_rating, max_item_id, max_global_idx)
        # print(user_df.iloc[max_idx])

    return user2item_id_m, test_total_global_idx_l, user2max_item_id_m, total_n_test_l


def get_sample_user_id(group_userid_df, n_sample_user, n_rating_thres):
    user_n_rating_df = group_userid_df.count()
    user_n_rating_df.rename(columns={'itemId': 'n_rating'}, inplace=True)
    user_n_rating_df.drop(columns=['rating'], inplace=True)
    user_n_rating_df.where(user_n_rating_df['n_rating'] > n_rating_thres, inplace=True)
    user_n_rating_df.dropna(inplace=True)

    if len(user_n_rating_df) < n_sample_user:
        print("warning: return user too small, use maximum number instead")
        n_sample_user = len(user_n_rating_df)
    rand_idx_l = np.random.permutation(len(user_n_rating_df))[:n_sample_user]
    user_n_rating_df = user_n_rating_df.iloc[rand_idx_l]
    sample_uid_l = user_n_rating_df.index.to_numpy()
    return sample_uid_l


def matrix_factorization(df, n_dim):
    n_user = np.max(df['userId']) + 1
    n_item = np.max(df['itemId']) + 1

    mf = libpmf.train_coo(row_idx=df['userId'], col_idx=df['itemId'],
                          obs_val=df['rating'],
                          m=n_user,
                          n=n_item, param_str='-k %d -n 15' % n_dim)
    user_l = mf['W']
    item_l = mf['H']
    return user_l, item_l


# hitting rate 关于dimensionality的曲线
if __name__ == '__main__':
    np.random.seed(0)
    filename_l = ['movielens-small', 'movielens-20m', 'movielens-25m', 'movielens-27m', 'netflix', 'amazon']
    # filename_l = ['movielens-small']
    base_dir = '/home/bianzheng/Dataset/Recommendation'
    n_dim_l = np.arange(10, 200, 10)
    # n_dim_l = [10]
    for fn in filename_l:
        csv_filename = os.path.join(base_dir, '%s-ratings.csv' % fn)
        train_l = get_train_set(csv_filename, fn)
        hr_l = []
        for n_dim in n_dim_l:
            hr = calc_hitting_rate(train_l, n_dim)
            hr_l.append(hr)
        draw_curve(n_dim_l, hr_l, fn)
