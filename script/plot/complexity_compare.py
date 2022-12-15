import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
from sklearn import linear_model

linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['x', "v", "o", "D", "s"]
markersize = 15

matplotlib.rcParams.update({'font.size': 15})


def lr(x_l, y_l):
    reg = linear_model.LinearRegression()
    new_x_l = np.array(x_l, dtype=np.float32)[:, np.newaxis]
    reg.fit(new_x_l, y_l)
    # pred_y_l = reg.predict(new_x_l)
    return [reg.coef_[0], reg.intercept_]


def plot_figure(*, fname: str, test: bool):
    n_query = 1000
    n_user = 1000000

    df = pd.read_csv(fname)
    binary_search_run_time_l = df['binary_search'].to_numpy() / n_query * 1000
    score_computation_run_time_l = df['score_computation'].to_numpy() / n_query * 1000
    binary_search_coef = lr(df['equal_dim'], binary_search_run_time_l)
    score_comp_coef = lr(df['equal_dim'], score_computation_run_time_l)
    print("binary search coef", binary_search_coef)
    print("score computation coef", score_comp_coef)

    subplot_str = 111
    fig = plt.figure(figsize=(6, 4))
    ax = fig.add_subplot(subplot_str)
    ax.scatter(df['equal_dim'], binary_search_run_time_l, label='Rank Bound Computation', c='#000000', s=50,
               marker=marker_l[0])
    ax.scatter(df['equal_dim'], score_computation_run_time_l, label='Score Computation', c='#000000', s=50,
               marker=marker_l[1])

    ax.plot(df['equal_dim'], df['equal_dim'] * binary_search_coef[0] + binary_search_coef[1],
            color='#000000', linewidth=2.5, linestyle='dotted')
    ax.plot(df['equal_dim'], df['equal_dim'] * score_comp_coef[0] + score_comp_coef[1],
            color='#000000', linewidth=2.5, linestyle='dotted')

    ax.set_xlabel(r"$d$ / $\log \tau$")
    # ax.set_xlim([0, 210])
    ax.set_ylabel('Query Time (ms)', labelpad=None)
    # ax.set_ylim(ylim)
    # ax.set_yscale('log')
    ax.legend(frameon=False, loc='best')
    if test:
        plt.savefig("complexity_compare.jpg", bbox_inches='tight', dpi=600)
    else:
        plt.savefig("complexity_compare.pdf", bbox_inches='tight')


if __name__ == '__main__':
    fname = 'data/binary_search_complexity/complexity.csv'
    plot_figure(fname=fname, test=False)
    pass
