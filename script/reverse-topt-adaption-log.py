import numpy as np
import pandas as pd

method_name = 'topt_ip'

df = pd.DataFrame([],
                  columns=['queryID', 'result_size', 'rtk_topt', 'query_time'],
                  dtype=np.int32)

with open('netflix-top10-reverse-topk-adaption.log', 'r') as f:
    for line in f:
        segment_l = line.split(" ")
        queryID = int(segment_l[-7][:-1])
        result_size = int(segment_l[-5][:-1])
        rtk_topt = int(segment_l[-3][:-1])
        query_time = float(segment_l[-1][:-2])

        new_df = pd.DataFrame([{'queryID': queryID,
                                'result_size': result_size, 'rtk_topt': rtk_topt, 'query_time': query_time}])
        df = pd.concat([df, new_df])

print(df)
print(np.sum(df['rtk_topt'] >= 2048))
print(len(df['rtk_topt'] >= 2048))
