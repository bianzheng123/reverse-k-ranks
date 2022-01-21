import numpy as np
import pandas as pd
import os
import sys
import matplotlib.pyplot as plt


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
    plt.xscale('log', base=2)
    plt.xticks(x)
    # plt.ticklabel_format(axis='x', style='sci')
    # plt.xlim(1, 500000)

    # 使用legend绘制多条曲线
    # plt.title('graph kmeans vs knn')
    # plt.legend(loc='upper left', title="legend")
    plt.title("dimensionality vs hitting rate curve")

    plt.xlabel("dimensionality")
    plt.ylabel("hitting rate")
    plt.grid(True, linestyle='-.')
    # plt.xticks([0, 0.1, 0.2, 0.3, 0.4])
    # plt.yticks([0.75, 0.8, 0.85])
    plt.savefig('result/hr_dim_curve/%s.jpg' % (name))
    plt.close()


if __name__ == '__main__':
    x_axis = np.array([2 ** i for i in range(3, 11, 1)])
    print(x_axis)
    y_axis = np.array([0.6329, 0.6843, 0.7031, 0.7020, 0.6950, 0.6776, 0.6707, 0.6616])
    draw_curve(x_axis, y_axis, 'movielens-1m')
