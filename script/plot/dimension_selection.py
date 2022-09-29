import json
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import os

params = {
    'axes.labelsize': 8,
    'font.size': 8,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'text.usetex': False,
    'figure.figsize': [4.5, 4.5]
}
linestyle_l = ['_', '-', '--', ':']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
marker_l = ['H', 'D', 'P', '>', '*', 'X', 's', '<', '^', 'p', 'v']
markersize = 10
matplotlib.RcParams.update(params)


def plot():
    fig, ax = plt.subplots()

    ax.plot(dim_l, hit_50_l,
            color=color_l[0], linewidth=2.5, linestyle='-',
            label='HR@50',
            marker=marker_l[0], markersize=markersize)
    ax.plot(dim_l, hit_100_l,
            color=color_l[1], linewidth=2.5, linestyle='-',
            label='HR@100',
            marker=marker_l[1], markersize=markersize)
    ax.plot(dim_l, hit_200_l,
            color=color_l[2], linewidth=2.5, linestyle='-',
            label='HR@200',
            marker=marker_l[2], markersize=markersize)
    ax.set_xlabel('dimensionality')
    ax.set_ylabel('Hitting Ratio')
    ax.legend(frameon=False, loc='lower right')

    plt.savefig("dimensionality_selection.jpg", bbox_inches='tight')
    plt.savefig("dimensionality_selection.pdf", bbox_inches='tight')
    plt.close()


if __name__ == "__main__":
    dim_l = [4, 8, 16, 32, 64, 128, 256]
    hit_50_l = []
    hit_100_l = []
    hit_200_l = []
    for dim in dim_l:
        with open('data/dimension_selection/hitting_rate-{}-new.json'.format(dim), 'r') as f:
            json_ins = json.load(f)
        hit_50_l.append(json_ins['test_result']['hit@50'])
        hit_100_l.append(json_ins['test_result']['hit@100'])
        hit_200_l.append(json_ins['test_result']['hit@200'])
    plot()
    print(hit_50_l)
    print(hit_100_l)
