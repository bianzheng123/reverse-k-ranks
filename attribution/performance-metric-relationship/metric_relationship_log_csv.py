import pandas as pd
import os
import numpy as np
import matplotlib.pyplot as plt


def plot(data_x, data_y, para, x_axis_name, y_axis_name, title_name, file_name):
    print("corr", np.corrcoef(data_x, data_y))

    # plot
    fig, ax = plt.subplots()

    # ax.scatter(x, y, s=sizes, c=colors, vmin=0, vmax=100)
    # ax.scatter(x, y, vmin=0, vmax=n_data_item)

    ax.plot(data_x, (data_x ** para[0]) * (10 ** para[1]), color='#ff0000')
    # ax.plot(data_x, (data_x * para[0]) + para[1])
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


def least_square_parameter(x, y):
    # log_x = x
    # log_y = y
    log_x = np.log10(x)
    log_y = np.log10(y)
    # assemble matrix A
    A = np.vstack([log_x, np.ones(len(log_x))]).T

    # turn log_y into a column vector
    log_y = np.array(log_y)
    log_y = log_y[:, np.newaxis]
    # Direct least square regression
    alpha = np.dot((np.dot(np.linalg.inv(np.dot(A.T, A)), A.T)), log_y)
    print(alpha)
    return alpha


def read_log(filename):
    queryID_l = []
    n_refine_l = []
    io_cost_l = []
    ip_cost_l = []
    total_running_time_l = []
    io_time_perc_l = []
    ip_time_perc_l = []

    io_time_l = []
    ip_time_l = []
    memory_index_time_l = []
    with open(filename, 'r') as f:
        lines = f.read().split('\n')
        for line in lines:
            if line == '':
                break
            split_l = line.split(' ')
            queryID = int(split_l[-24][:-1])
            queryID_l.append(queryID)

            n_refine = int(split_l[-22][:-1])
            n_refine_l.append(n_refine)

            io_cost = int(split_l[-20][:-1])
            io_cost_l.append(io_cost)

            ip_cost = int(split_l[-18][:-1])
            ip_cost_l.append(ip_cost)

            inner_product_time = float(split_l[-14][:-2])
            rank_compute_time = float(split_l[-1][:-1])
            read_disk_time = float(split_l[-5][:-2])
            memory_index_search_time = float(split_l[-9][:-2])
            running_time = rank_compute_time + read_disk_time + memory_index_search_time + inner_product_time
            total_running_time_l.append(running_time)

            io_time_perc = read_disk_time / running_time
            io_time_perc_l.append(io_time_perc)

            ip_time_perc = rank_compute_time / running_time
            ip_time_perc_l.append(ip_time_perc)

            io_time_l.append(read_disk_time)
            ip_time_l.append(rank_compute_time)
            memory_index_time_l.append(memory_index_search_time + inner_product_time)

    print("{} io_time {}s, ip_time {}s, memory_index_time {}s".format(filename, np.average(io_time_l), np.average(ip_time_l), np.average(memory_index_time_l)))
            # print(rank_compute_time, read_disk_time, memory_index_search_time, inner_product_time, n_user_candidate)
    assert len(queryID_l) == len(n_refine_l) == len(io_cost_l) == len(ip_cost_l) == len(io_time_perc_l) == len(
        ip_time_perc_l) == len(total_running_time_l)
    data_df = {'queryID': queryID_l, 'n_refine': n_refine_l, 'io_cost': io_cost_l, 'ip_cost': ip_cost_l,
               'total_running_time': total_running_time_l,
               'io_time_perc': io_time_perc_l, 'ip_time_perc': ip_time_perc_l}
    df = pd.DataFrame(data_df)
    return df


if __name__ == '__main__':
    # dataset_m = {'yahoomusic_big': 'Yahoomusic', 'yelp': 'Yelp', 'fakebig': 'fakebig'}
    dataset_m = {'fakebig': 'fakebig'}
    # dataset_m = {'yahoomusic_big': 'Yahoomusic'}
    for ds in dataset_m.keys():
        basic_dir = "../../result"
        fname = "{}-RSCompressTopTIP-top10.log".format(ds)
        df = read_log(os.path.join(basic_dir, fname))

        # user_candidate_l = df['n_refine']
        # running_time_l = df['total_running_time']
        # para = least_square_parameter(user_candidate_l, running_time_l)
        # plot(user_candidate_l, running_time_l, para, "Number of Refinement", 'Total Running Time (second)',
        #      '# Refinement vs Total Running Time in {}'.format(dataset_m[ds]), '{}-Refinement-RunTime'.format(ds))
        #
        # io_cost_l = df['io_cost']
        # para = least_square_parameter(user_candidate_l, io_cost_l)
        # plot(user_candidate_l, io_cost_l, para,
        #      "Number of Refinement", 'IO cost',
        #      '{} vs {} in {}'.format('# Refinement', 'IO cost', dataset_m[ds]),
        #      '{}-Refinement-IOCost'.format(ds))
        #
        # ip_cost_l = df['ip_cost']
        # para = least_square_parameter(user_candidate_l, ip_cost_l)
        # plot(user_candidate_l, ip_cost_l, para,
        #      "Number of Refinement", "IP cost",
        #      '{} vs {} in {}'.format('# Refinement', 'IP cost', dataset_m[ds]),
        #      '{}-Refinement-IPCost'.format(ds))
        #
        # df.rename({'total_running_time': 'total_running_time(second)'}, inplace=True)
        # df.to_csv('{}-top-10.csv'.format(ds), float_format="%.2f", index=False)
