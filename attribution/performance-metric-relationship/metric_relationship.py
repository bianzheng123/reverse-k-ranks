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


if __name__ == '__main__':
    dataset_m = {'movielens-27m': 'Movielens', 'netflix': 'Netflix', 'yahoomusic_big': 'Yahoomusic'}
    # dataset_m = {'yahoomusic_big': 'Yahoomusic'}
    dataset_info_m = {'movielens-27m': [283228, 53889], 'netflix': [480189, 17770], 'yahoomusic_big': [1823179, 135736]}
    # dataset_l = ['movielens-27m', 'netflix', 'yahoomusic_big']
    for ds in dataset_m.keys():
        basic_dir = "../../result/attribution/PerformanceMetricRelationship"
        fname = "{}-RSCompressTopTIPBruteForce-top10-n_sample_2048-index_size_gb_256.txt".format(ds)
        df = pd.read_csv(os.path.join(basic_dir, fname), dtype=np.int64, skipinitialspace=True)

        prune_ratio = df['prune_ratio']
        io_cost = df['io_cost']
        ip_cost = df['ip_cost']
        plot(prune_ratio, io_cost,
             "Number of Refinement", 'IO cost',
             '{} vs {} in {}'.format('# Refinement', 'IO cost', dataset_m[ds]),
             '{}-PruneRatio-IOCost'.format(ds))
        plot(prune_ratio, ip_cost + dataset_info_m[ds][0],
             "Number of Refinement", "IP cost",
             '{} vs {} in {}'.format('# Refinement', 'IP cost', dataset_m[ds]),
             '{}-PruneRatio-IPCost'.format(ds))
        # print(df)
