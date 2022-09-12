import matplotlib.pyplot as plt
import numpy as np
from matplotlib import ticker
import matplotlib

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

label_m = {'movielens-27m': 'Movielens', 'netflix': 'Netflix', 'yahoomusic_big': 'Yahoomusic', 'yelp': 'Yelp'}
label_l = ['movielens-27m', 'netflix']
io_time_l = [4899.685, 21183.491]
memory_index_time_l = [48.105, 70.649]

io_time_l = np.array(io_time_l)
memory_index_time_l = np.array(memory_index_time_l)

total_time_l = io_time_l + memory_index_time_l

io_time_perc_l = io_time_l / total_time_l
memory_index_time_perc_l = memory_index_time_l / total_time_l

width = 0.35  # the width of the bars: can also be len(x) sequence

fig, ax = plt.subplots()
ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1, decimals=0))
# ax.set_yscale('log')

ax.bar(label_l, memory_index_time_perc_l, width, color='#080604', edgecolor='#000000',
       hatch='//', label='Memory Index Time')
ax.bar(label_l, io_time_perc_l, width, bottom=memory_index_time_perc_l, color='#ffffff', edgecolor='#000000',
       hatch='', label='Disk Index IO Time')

# ax.set_yscale('log')

ax.set_ylabel('Percentage of Running Time')
ax.legend(frameon=False, loc='best')

# plt.savefig("running_time_profile.pdf", bbox_inches='tight')
plt.savefig("running_time_profile.jpg", dpi=600, bbox_inches='tight')
