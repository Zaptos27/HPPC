import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

sns.set_style("dark")

data_euler = np.loadtxt('week1/SIR_out_euler.txt',skiprows=1).T
data_RK45 = np.loadtxt('week1/SIR_out_RK45.txt',skiprows=1).T


plt.figure()
plt.plot(data_euler[0],data_euler[1],label = 'S')
plt.plot(data_euler[0],data_euler[2],label = 'I')
plt.plot(data_euler[0],data_euler[3],label = 'R')
plt.plot(data_euler[0],np.sum(data_euler[1:],axis=1),label = 'R')
plt.show()

