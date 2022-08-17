import pandas as pd
import os
import numpy as np
import matplotlib.pyplot as plt


def plot(data_x, data_y, x_axis_name, y_axis_name, title_name, file_name):
    print("corr", np.corrcoef(data_x, data_y))

    # plot
    fig, ax = plt.subplots()

    # ax.scatter(x, y, s=sizes, c=colors, vmin=0, vmax=100)
    # ax.scatter(x, y, vmin=0, vmax=n_data_item)
    ax.scatter(data_x, data_y, s=2)

    # ax.set(xlim=(0, n_data_item),
    #        ylim=(0, n_data_item))
    # ax.set(xlim=(0, 5000),
    #        ylim=(0, 5000))

    ax.set_xlabel(x_axis_name)
    ax.set_ylabel(y_axis_name)
    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_title(title_name)

    plt.savefig('{}.jpg'.format(file_name), dpi=600, bbox_inches='tight')
    plt.close()


def read_log(filename):
    total_running_time_l = []
    n_user_candidate_l = []
    with open(filename, 'r') as f:
        lines = f.read().split('\n')
        for line in lines:
            if line == '':
                break
            split_l = line.split(' ')
            rank_compute_time = float(split_l[-1][:-1])
            read_disk_time = float(split_l[-5][:-2])
            memory_index_search_time = float(split_l[-9][:-2])
            inner_product_time = float(split_l[-14][:-2])
            n_user_candidate = int(split_l[-22][:-1])
            running_time = rank_compute_time + read_disk_time + memory_index_search_time + inner_product_time
            total_running_time_l.append(running_time)
            n_user_candidate_l.append(n_user_candidate)
            # print(rank_compute_time, read_disk_time, memory_index_search_time, inner_product_time, n_user_candidate)
    assert len(total_running_time_l) == len(n_user_candidate_l)
    return total_running_time_l, n_user_candidate_l


if __name__ == '__main__':
    dataset_m = {'yahoomusic_big': 'Yahoomusic', 'yelp': 'Yelp'}
    for ds in dataset_m.keys():
        basic_dir = "../../result"
        fname = "{}-RSCompressTopTIP-top10.log".format(ds)
        running_time_l, user_candidate_l = read_log(os.path.join(basic_dir, fname))

        plot(user_candidate_l, running_time_l, "Number of Refinement", 'Total Running Time (second)',
             '# Refinement vs Total Running Time in {}'.format(dataset_m[ds]), '{}-Refinement-RunTime'.format(ds))
