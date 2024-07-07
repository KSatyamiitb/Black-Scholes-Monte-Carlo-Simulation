import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

numTimePeriods = int(sys.argv[1])
numSimulations = int(sys.argv[2])
filename = './simulations/sim_'+sys.argv[3]+'.csv'
data = pd.read_csv(filename,header=None)
filename = './plots/plot_'+sys.argv[3]+'.png'
x = np.arange(0,numTimePeriods+1)
for i in range(numSimulations):
    plt.plot(x,data.iloc[i,:])
plt.xlabel('Time Periods')
plt.ylabel('Underlying Price')
plt.title('Plot')
plt.savefig(filename)
