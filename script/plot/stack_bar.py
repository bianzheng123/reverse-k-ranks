import matplotlib.pyplot as plt
import numpy as np

labels = ['G1', 'G2', 'G3', 'G4', 'G5']
men_means = [20, 34, 30, 35, 27, 25, 32, 34, 20, 25]

x = np.arange(len(labels))  # the label locations
width = 0.35  # the width of the bars

print(np.append(x - width / 2, x + width / 2))

fig, ax = plt.subplots()
rects2 = ax.bar(np.append(x - width / 2, x + width / 2), [10] * 10, width - 0.1, label='IO Time')
rects1 = ax.bar(np.append(x - width / 2, x + width / 2), men_means, width - 0.1, bottom=[10] * 10, label='IP Time')

# rects3 = ax.bar(x + width / 2, [10] * 5, width, label='IO Time')
# rects4 = ax.bar(x + width / 2, women_means, width, bottom=[10] * 5, label='IP Time')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Scores')
ax.set_title('Scores by group and gender')
ax.set_xticks(x, labels)
ax.legend()

ax.bar_label(rects1, padding=3)
ax.bar_label(rects2, padding=3)

fig.tight_layout()

plt.savefig('stack.jpg', dpi=600, bbox_inches='tight')
