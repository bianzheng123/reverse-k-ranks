from scipy.stats import norm
import matplotlib.pyplot as plt


def plot(x_l, y_l, fname):
    plt.plot(x_l, y_l,
             color='#3D0DFF', linewidth=2.5, linestyle='-',
             label='Query Rank Sample',
             marker='H', markersize=2)
    plt.xlabel('sampled rank')
    # ax.set_ylabel('Running Time (ms)')
    # ax.set_ylim(0)
    # plt.yscale('log')
    plt.ylabel('IP')
    plt.title('ip_plot')
    plt.savefig(fname, dpi=600)
    plt.close()


n_point = 1000
x_l = [i / n_point for i in range(n_point)]
y_l = [norm.ppf(x) for x in x_l]
plot(x_l, y_l, "inverse-normal-cdf.jpg")
