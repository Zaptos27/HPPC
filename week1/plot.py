import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

sns.set_style("darkgrid")
plt.rcParams.update({'font.size': 22})
data_euler = np.loadtxt('SIR_out_euler.txt',skiprows=1).T
data_RK45 = np.loadtxt('SIR_out_RK45.txt',skiprows=1).T
data_RK45_r = np.loadtxt('SIR_out_RK45_random.txt',skiprows=1).T



plt.figure(figsize=(14,10))
plt.title('SIR numerically solve with the euler method')
plt.plot(data_euler[0],data_euler[1],label = 'S')
plt.plot(data_euler[0],data_euler[2],label = 'I')
plt.plot(data_euler[0],data_euler[3],label = 'R')
plt.plot(data_euler[0],np.sum(data_euler[1:],axis=0),label = 'Total')
plt.xlabel('Time [Day]')
plt.ylabel('# of people')
plt.legend()
plt.tight_layout()
plt.show()

plt.figure(figsize=(14,10))
plt.title('SIR numerically solve with the RK45')
plt.plot(data_RK45[0],data_RK45[1],label = 'S')
plt.plot(data_RK45[0],data_RK45[2],label = 'I')
plt.plot(data_RK45[0],data_RK45[3],label = 'R')
plt.plot(data_RK45[0],np.sum(data_RK45[1:],axis=0),label = 'Total')
plt.xlabel('Time [Day]')
plt.ylabel('# of people')
plt.legend()
plt.tight_layout()
plt.show()

plt.figure(figsize=(14,10))
plt.title('SIR numerically solve with the RK45 with Random pertubations')
plt.plot(data_RK45_r[0],data_RK45_r[1],label = 'S')
plt.plot(data_RK45_r[0],data_RK45_r[2],label = 'I')
plt.plot(data_RK45_r[0],data_RK45_r[3],label = 'R')
plt.plot(data_RK45_r[0],np.sum(data_RK45_r[1:],axis=0),label = 'Total')
plt.xlabel('Time [Day]')
plt.ylabel('# of people')
plt.legend()
plt.tight_layout()
plt.show()


print(np.where(np.isclose(np.sum(data_euler[1:],axis=0),1000)==False))
print(np.where(np.isclose(np.sum(data_RK45[1:],axis=0),1000)==False))
print(np.where(np.isclose(np.sum(data_RK45_r[1:],axis=0),1000)==False))

# Check monotone

#Do the math




