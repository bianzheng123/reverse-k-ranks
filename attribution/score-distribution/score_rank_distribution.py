import numpy as np
import matplotlib.pyplot as plt


def plot_rank_pdf(score_distri, idx):
    # plot
    fig, ax = plt.subplots()

    n_data_item = len(score_distri)
    rank_pdf_x = np.arange(1, n_data_item + 1, 1)
    rank_pdf_y = score_distri

    # ax.scatter(x, y, s=sizes, c=colors, vmin=0, vmax=100)
    # ax.scatter(x, y, vmin=0, vmax=n_data_item)
    ax.plot(rank_pdf_x, rank_pdf_y)

    ax.set(xlim=(1, n_data_item),
           ylim=(score_distri[n_data_item - 1], score_distri[0]))
    # ax.set(xlim=(0, 5000),
    #        ylim=(0, 5000))

    ax.set_xlabel('rank')
    ax.set_ylabel('score')
    ax.set_title('movielens-27m, n_data_item={}'.format(n_data_item))

    # plt.show()
    plt.savefig('rank_pdf_{}.jpg'.format(idx))
    plt.close()


def plot_score_distribution(score_distri, idx):
    fig, ax = plt.subplots()

    ax.hist(score_distri, bins=16, linewidth=0.5, edgecolor="white")

    # ax.set(xlim=(0, 8), xticks=np.arange(1, 8),
    #        ylim=(0, 56), yticks=np.linspace(0, 56, 9))
    ax.set_xlabel('score')
    ax.set_ylabel('frequency')
    ax.set_title('movielens-27m')

    # plt.show()
    plt.savefig('score_distribution_{}.jpg'.format(idx))
    plt.close()


def plot_all_score_distribution(score_distribution_l):
    fig, ax = plt.subplots()
    total_size = len(score_distribution_l) * len(score_distribution_l[0])
    all_score = score_distribution_l.reshape(total_size)

    ax.hist(all_score, bins=16, linewidth=0.5, edgecolor="white")

    # ax.set(xlim=(0, 8), xticks=np.arange(1, 8),
    #        ylim=(0, 56), yticks=np.linspace(0, 56, 9))
    ax.set_xlabel('score')
    ax.set_ylabel('frequency')
    ax.set_title('movielens-27m')

    # plt.show()
    plt.savefig('all_score_distribution.jpg')
    plt.close()


if __name__ == '__main__':
    score_distribution_l = np.loadtxt(
        '/home/bianzheng/reverse-k-ranks/result/attribution/user-score-distribution-fake-normal.csv',
        delimiter=',')
    # sample_len = len(score_distribution_l)
    sample_len = 1
    plot_all_score_distribution(score_distribution_l)
    for i in range(sample_len):
        plot_rank_pdf(score_distribution_l[i], i)
        plot_score_distribution(score_distribution_l[i], i)
