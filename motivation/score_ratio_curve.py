import numpy as np
import faiss
import vecs_io
import matplotlib.pyplot as plt


def show_curve(x_l_l, y_l_l, dataset_name_l, filename, ylabel):
    # 第一个是横坐标的值，第二个是纵坐标的值
    # plt.figure(num=3, figsize=(8, 5))
    # marker
    # o 圆圈, v 倒三角, ^ 正三角, < 左三角, > 右三角, 8 大圆圈, s 正方形, p 圆圈, * 星号, h 菱形, H 六面体, D 大菱形, d 瘦的菱形, P 加号, X 乘号
    # 紫色#b9529f 蓝色#3953a4 红色#ed2024 #231f20 深绿色#098140 浅绿色#7f8133 #0084ff
    # solid dotted

    marker_l = ['H', 'D', 'P', '>', '*', 'X', 's', '<', '^', 'p', 'v']
    color_l = ['#b9529f', '#3953a4', '#ed2024', '#098140', '#231f20', '#7f8133', '#0084ff']
    lenx = len(x_l_l)
    for i in range(lenx):
        plt.plot(x_l_l[i], y_l_l[i], marker=marker_l[i], linestyle='solid',
                 color=color_l[i],
                 label=dataset_name_l[i])
    # plt.xscale('log')
    # plt.xlim(1, 500000)

    # 使用legend绘制多条曲线
    plt.title('dimensionality as score ratio')
    plt.legend(loc='upper left')

    plt.xlabel('dimension')
    plt.ylabel(ylabel)
    plt.grid(True, linestyle='-.')
    # plt.xticks([0, 0.1, 0.2, 0.3, 0.4])
    # plt.yticks([0.75, 0.8, 0.85])
    plt.savefig('result/%s.jpg' % filename)
    plt.close()


def ip_gnd(base, query, k):
    base_dim = base.shape[1]
    index = faiss.IndexFlatIP(base_dim)
    index.add(base)
    gnd_distance, gnd_idx = index.search(query, k)
    return gnd_idx, gnd_distance


if __name__ == '__main__':
    # item_name_l = ['correlated', 'independent']
    item_name_l = ['movielens-27m', 'random', 'netflix']
    # n_dim_l = [2, 3, 4, 5, 10, 15, 20, 25, 30]
    # n_dim_l = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150]
    n_dim_l = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    # n_dim_l = [10]

    curve_y_l = []
    for i, item_name in enumerate(item_name_l, 0):
        tmp_y_l = []
        for eval_dim in n_dim_l:
            user_l, dim = vecs_io.fvecs_read(
                '/home/bianzheng/Dataset/MIPS/user_item/dataset-dimension/%s-%dd/%s_user.fvecs' % (
                    item_name, eval_dim, item_name))
            item_l, dim = vecs_io.fvecs_read(
                '/home/bianzheng/Dataset/MIPS/user_item/dataset-dimension/%s-%dd/%s_item.fvecs' % (
                    item_name, eval_dim, item_name))

            rand_idx_l = np.random.permutation(len(user_l))
            rand_idx_l = rand_idx_l[:10000]
            user_l = user_l[rand_idx_l, :]

            # user_l = np.abs(user_l)
            # item_l = np.abs(item_l)

            print(item_name, eval_dim)

            max_idx_l, max_dist_l = ip_gnd(item_l, user_l, 1)
            min_idx_l, min_dist_l = ip_gnd(-item_l, user_l, 1)
            min_dist_l = -min_dist_l
            # print(min_dist_l)
            min_dist_l = min_dist_l[:, 0]

            # print(np.argmin(res_matrix, axis=1))
            # print(max_dist_l, min_dist_l)

            val_l = (max_dist_l - min_dist_l) / eval_dim
            # val_l = max_dist_l - min_dist_l
            # val_l = max_dist_l / min_dist_l

            avg_val = np.average(val_l)
            tmp_y_l.append(avg_val)
        curve_y_l.append(tmp_y_l)
        np.savetxt('result/diff_per_dim-%s.txt' % item_name, tmp_y_l, fmt="%.3f")
    curve_x_l = np.tile(n_dim_l, (len(item_name_l), 1))
    show_curve(curve_x_l, curve_y_l, item_name_l, 'diff_per_dim', '(maxscore - minscore) / dimension')

    curve_y_l = []
    for i, item_name in enumerate(item_name_l, 0):
        tmp_y_l = []
        for eval_dim in n_dim_l:
            user_l, dim = vecs_io.fvecs_read(
                '/home/bianzheng/Dataset/MIPS/user_item/dataset-dimension/%s-%dd/%s_user.fvecs' % (
                    item_name, eval_dim, item_name))
            item_l, dim = vecs_io.fvecs_read(
                '/home/bianzheng/Dataset/MIPS/user_item/dataset-dimension/%s-%dd/%s_item.fvecs' % (
                    item_name, eval_dim, item_name))

            rand_idx_l = np.random.permutation(len(user_l))
            rand_idx_l = rand_idx_l[:10000]
            user_l = user_l[rand_idx_l, :]

            # user_l = np.abs(user_l)
            # item_l = np.abs(item_l)

            print(item_name, eval_dim)

            max_idx_l, max_dist_l = ip_gnd(item_l, user_l, 1)
            min_idx_l, min_dist_l = ip_gnd(-item_l, user_l, 1)
            min_dist_l = -min_dist_l
            # print(min_dist_l)
            min_dist_l = min_dist_l[:, 0]

            # print(np.argmin(res_matrix, axis=1))
            # print(max_dist_l, min_dist_l)

            # val_l = (max_dist_l - min_dist_l) / eval_dim
            val_l = max_dist_l - min_dist_l
            # val_l = max_dist_l / min_dist_l

            avg_val = np.average(val_l)
            tmp_y_l.append(avg_val)
        curve_y_l.append(tmp_y_l)
        np.savetxt('result/diff_total-%s.txt' % item_name, tmp_y_l, fmt="%.3f")
    curve_x_l = np.tile(n_dim_l, (len(item_name_l), 1))
    show_curve(curve_x_l, curve_y_l, item_name_l, 'diff_total', 'maxscore - minscore')
