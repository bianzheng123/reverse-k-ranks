import numpy as np
import matplotlib.pyplot as plt


def plot(x_l, y_l, z_l):
    # plot
    fig, ax = plt.subplots()

    ax.plot(x_l, y_l, linestyle='solid', color='#000000', label="distribution")
    ax.plot(x_l, z_l, linestyle='solid', color='#555555', label="distribution")

    ax.legend(loc='upper left')

    # ax.set(xlim=(-2000, n_data_item + 2000),
    #        ylim=(score_distri[n_data_item - 1] - 2, score_distri[0] + 2))
    # ax.set(xlim=(0, 5000),
    #        ylim=(0, 5000))

    ax.set_xlabel('rank')
    ax.set_ylabel('score')
    # ax.set_title('movielens-27m, n_data_item={}'.format(n_data_item))

    # plt.show()
    plt.savefig('false-positive-curve.jpg'.format(1), dpi=600, bbox_inches='tight')
    plt.close()


def f(m, k, n):
    fp = (1 - np.exp(-k * n / m)) ** k
    return fp


n_user = 6218834
n_data_item = 2166750

m_l = np.arange(1, n_user, 1000)
n = n_user
k = np.ceil(np.log(2) * n / m)
k = 6

fp_l = [f(m, k, n) for m in m_l]
plot(m_l, fp_l)
