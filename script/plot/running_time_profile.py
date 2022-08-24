import matplotlib.pyplot as plt
import numpy as np
from matplotlib import ticker
import matplotlib


def read_log_average(filename):
    queryID_l = []
    n_refine_l = []
    io_cost_l = []
    ip_cost_l = []
    total_running_time_l = []
    io_time_l = []
    ip_time_l = []
    memory_index_time_l = []
    with open(filename, 'r') as f:
        lines = f.read().split('\n')
        for line in lines:
            if line == '':
                break
            split_l = line.split(' ')
            queryID = int(split_l[-16][:-1])
            queryID_l.append(queryID)

            n_refine = int(split_l[-14][:-1])
            n_refine_l.append(n_refine)

            io_cost = int(split_l[-12][:-1])
            io_cost_l.append(io_cost)

            ip_cost = int(split_l[-10][:-1])
            ip_cost_l.append(ip_cost)

            total_time = float(split_l[-7][:-2])
            total_running_time_l.append(total_time)

            ip_time = float(split_l[-4][:-2])
            ip_time_l.append(ip_time)

            io_time = float(split_l[-1][:-1])
            io_time_l.append(io_time)

            memory_index_time_l.append(total_time - io_time - ip_time)

            # print(rank_compute_time, read_disk_time, memory_index_search_time, inner_product_time, n_user_candidate)
    assert len(queryID_l) == len(n_refine_l) == len(io_cost_l) == len(ip_cost_l) == len(total_running_time_l)
    # data_df = {'queryID': queryID_l, 'n_refine': n_refine_l, 'io_cost': io_cost_l, 'ip_cost': ip_cost_l,
    #            'total_running_time': total_running_time_l,
    #            'io_time_perc': io_time_perc_l, 'ip_time_perc': ip_time_perc_l}
    avg_ip_time = np.average(ip_time_l)
    avg_io_time = np.average(io_time_l)
    avg_memory_index_time = np.average(memory_index_time_l)
    return avg_ip_time, avg_io_time, avg_memory_index_time


params = {
    'axes.labelsize': 8,
    'font.size': 8,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'text.usetex': False,
    'figure.figsize': [4.5, 4.5]
}
hatch = ['--', '+', 'x', '\\']
color_l = ['#3D0DFF', '#6BFF00', '#00E8E2', '#EB0225', '#FF9E03']
matplotlib.RcParams.update(params)

label_m = {'yahoomusic_big': 'Yahoomusic', 'yelp': 'Yelp'}
label_l = []
ip_time_l = []
io_time_l = []
memory_index_time_l = []

for dataset in label_m:
    avg_ip_time, avg_io_time, avg_memory_index_time = read_log_average(
        'data/{}-RSCompressTopTIP-top10.log'.format(dataset))
    label_l.append(label_m[dataset])
    ip_time_l.append(avg_ip_time)
    io_time_l.append(avg_io_time)
    memory_index_time_l.append(avg_memory_index_time)

ip_time_l = np.array(ip_time_l)
io_time_l = np.array(io_time_l)
memory_index_time_l = np.array(memory_index_time_l)

total_time_l = ip_time_l + io_time_l + memory_index_time_l

ip_time_perc_l = ip_time_l / total_time_l
io_time_perc_l = io_time_l / total_time_l
memory_index_time_perc_l = memory_index_time_l / total_time_l

width = 0.35  # the width of the bars: can also be len(x) sequence

fig, ax = plt.subplots()
ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1, decimals=0))

ax.bar(label_l, memory_index_time_perc_l, width, color='#080604', edgecolor='#000000',
       hatch='//', label='Memory Index Time')
ax.bar(label_l, io_time_perc_l, width, bottom=memory_index_time_perc_l, color='#ffffff', edgecolor='#000000',
       hatch='\\\\', label='Disk Index IO Time')
ax.bar(label_l, ip_time_perc_l, width, bottom=memory_index_time_perc_l + io_time_perc_l, color='#ffffff',
       edgecolor='#000000',
       hatch='', label='Disk Index IP Time')

# ax.set_yscale('log')

ax.set_ylabel('Percentage of Running Time')
ax.legend(frameon=False, loc='best')

# plt.savefig("running_time_profile.pdf", bbox_inches='tight')
plt.savefig("running_time_profile.jpg", dpi=600, bbox_inches='tight')
