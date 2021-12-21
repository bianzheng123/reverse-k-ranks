import numpy as np
import faiss
import vecs_io
import matplotlib.pyplot as plt


def show_curve(x_l, y_l, dataset_name):
    # 第一个是横坐标的值，第二个是纵坐标的值
    # plt.figure(num=3, figsize=(8, 5))
    # marker
    # o 圆圈, v 倒三角, ^ 正三角, < 左三角, > 右三角, 8 大圆圈, s 正方形, p 圆圈, * 星号, h 菱形, H 六面体, D 大菱形, d 瘦的菱形, P 加号, X 乘号
    # 紫色#b9529f 蓝色#3953a4 红色#ed2024 #231f20 深绿色#098140 浅绿色#7f8133 #0084ff
    # solid dotted

    marker_l = ['H', 'D', 'P', '>', '*', 'X', 's', '<', '^', 'p', 'v']
    color_l = ['#b9529f', '#3953a4', '#ed2024', '#098140', '#231f20', '#7f8133', '#0084ff']
    plt.plot(x_l, y_l, marker=marker_l[0], linestyle='solid',
             color=color_l[0],
             label=dataset_name)
    # plt.xscale('log')
    # plt.xlim(1, 500000)

    # 使用legend绘制多条曲线
    plt.title('dimensionality as score ratio')
    plt.legend(loc='upper left')

    plt.xlabel('dimension')
    plt.ylabel('maxscore / minscore')
    plt.grid(True, linestyle='-.')
    # plt.xticks([0, 0.1, 0.2, 0.3, 0.4])
    # plt.yticks([0.75, 0.8, 0.85])
    plt.savefig('result/%s.jpg' % dataset_name)
    plt.close()


def ip_gnd(base, query, k):
    base_dim = base.shape[1]
    index = faiss.IndexFlatIP(base_dim)
    index.add(base)
    gnd_distance, gnd_idx = index.search(query, k)
    return gnd_idx, gnd_distance


if __name__ == '__main__':
    # item_name_l = ['correlated', 'independent']
    item_name_l = ['netflix']
    # n_dim_l = [2, 3, 4, 5, 10, 15, 20, 25, 30]
    n_dim_l = [10, 15, 20, 25, 30, 40, 50, 100, 150, 200, 250, 300]
    for i, item_name in enumerate(item_name_l, 0):
        print(item_name)
        user_l, dim = vecs_io.fvecs_read('data/%s/%s_user.fvecs' % (item_name, item_name))
        item_l, dim = vecs_io.fvecs_read('data/%s/%s_item.fvecs' % (item_name, item_name))

        rand_idx_l = np.random.permutation(len(user_l))
        rand_idx_l = rand_idx_l[:20000]
        user_l = user_l[rand_idx_l, :]

        user_l = np.abs(user_l)
        item_l = np.abs(item_l)

        curve_y_l = []
        for eval_dim in n_dim_l:
            print(item_name, eval_dim)
            tmp_item_l = item_l[:, :eval_dim].astype(np.float32)
            tmp_user_l = user_l[:, :eval_dim].astype(np.float32)

            max_idx_l, max_dist_l = ip_gnd(tmp_item_l, tmp_user_l, 1)
            min_idx_l, min_dist_l = ip_gnd(-tmp_item_l, tmp_user_l, 3)
            min_dist_l = -min_dist_l
            # print(min_dist_l)
            min_dist_l = min_dist_l[:, 0]

            # print(np.argmin(res_matrix, axis=1))
            # print(max_dist_l, min_dist_l)
            ratio_l = max_dist_l / min_dist_l
            avg_ratio = np.average(ratio_l)
            curve_y_l.append(avg_ratio)
        np.savetxt('result/ratio-%s.txt' % item_name, curve_y_l, fmt="%.3f")
        curve_x_l = n_dim_l
        show_curve(curve_x_l, curve_y_l, item_name)
