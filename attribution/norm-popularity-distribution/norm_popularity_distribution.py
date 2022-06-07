import matplotlib.pyplot as plt
import numpy as np

# movielens
# n_user = 283228
# n_data_item = 53889
# fake-normal
n_user = 1000
n_data_item = 5000
# netflix
# n_user = 480189
# n_data_item = 17770
# yelp-small
# n_user = 500000
# n_data_item = 50000

if __name__ == '__main__':
    for top_perc in [1, 2, 5, 8, 10]:
        distribution_l = np.loadtxt(
            '../../result/attribution/norm-popularity-distribution-fake-normal-perc-{}.csv'.format(top_perc),
            delimiter=',')
        scatter_x = distribution_l[:, 0]
        scatter_y = distribution_l[:, 1]

        # plot
        fig, ax = plt.subplots()

        # ax.scatter(x, y, s=sizes, c=colors, vmin=0, vmax=100)
        # ax.scatter(x, y, vmin=0, vmax=n_data_item)
        ax.scatter(scatter_x, scatter_y, s=2)

        # ax.set(xlim=(0, n_data_item),
        #        ylim=(0, n_data_item))
        # ax.set(xlim=(0, 5000),
        #        ylim=(0, 5000))

        ax.set_xlabel('norm')
        ax.set_ylabel('top {}% rank frequency'.format(top_perc))
        ax.set_title('movielens-27m, n_user: {}, n_data_item {}'.format(n_user, n_data_item))

        # plt.show()
        plt.savefig('norm-popularity-distribution-top-perc-{}.jpg'.format(top_perc))
        plt.close()
