import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

sns.set_style("darkgrid")
plt.rcParams.update({'font.size': 22})
data_euler = np.loadtxt('SIR_out_euler.txt',skiprows=1).T
data_RK45 = np.loadtxt('SIR_out_RK4.txt',skiprows=1).T
data_RK45_r = np.loadtxt('SIR_out_RK4_random.txt',skiprows=1).T
data_RK45_v = np.loadtxt('SIR_out_RK4_recoverd.txt',skiprows=1).T


plt.figure(figsize=(14,10))
plt.title('SIR numerically solve with the RK4 with recovered')
plt.plot(data_RK45_v[0],data_RK45_v[1],label = 'S')
plt.plot(data_RK45_v[0],data_RK45_v[2],label = 'I')
plt.plot(data_RK45_v[0],data_RK45_v[3],label = 'R')
plt.plot(data_RK45_v[0],np.sum(data_RK45_v[1:],axis=0),label = 'Total')
plt.xlabel('Time [Day]')
plt.ylabel('# of people')
plt.legend()
plt.tight_layout()
plt.savefig('RK4_recoverd.eps', dpi=192)
plt.show()


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
plt.savefig('euler_recoverd.eps', dpi=192)
plt.show()

plt.figure(figsize=(14,10))
plt.title('SIR numerically solve with the RK4')
plt.plot(data_RK45[0],data_RK45[1],label = 'S')
plt.plot(data_RK45[0],data_RK45[2],label = 'I')
plt.plot(data_RK45[0],data_RK45[3],label = 'R')
plt.plot(data_RK45[0],np.sum(data_RK45[1:],axis=0),label = 'Total')
plt.xlabel('Time [Day]')
plt.ylabel('# of people')
plt.legend()
plt.tight_layout()
plt.savefig('RK4_normal.eps', dpi=192)
plt.show()

plt.figure(figsize=(14,10))
plt.title('SIR numerically solve with the RK4 with Random pertubations')
plt.plot(data_RK45_r[0],data_RK45_r[1],label = 'S')
plt.plot(data_RK45_r[0],data_RK45_r[2],label = 'I')
plt.plot(data_RK45_r[0],data_RK45_r[3],label = 'R')
plt.plot(data_RK45_r[0],np.sum(data_RK45_r[1:],axis=0),label = 'Total')
plt.xlabel('Time [Day]')
plt.ylabel('# of people')
plt.legend()
plt.tight_layout()
plt.savefig('RK4_random.eps', dpi=192)
plt.show()

#Checking if close to 1000
print(np.where(np.isclose(np.sum(data_euler[1:],axis=0),1000)==False))
print(np.where(np.isclose(np.sum(data_RK45[1:],axis=0),1000)==False))
print(np.where(np.isclose(np.sum(data_RK45_r[1:],axis=0),1000)==False))

#Two ways of doing integral checking if int\gammaI = R
print(np.sum((data_euler[2]+np.roll(data_euler[2],-1))[:-1])*0.01/2*0.1,data_euler[3,-1])
print(0.01/2*(np.sum(data_euler[2, 1:-1])*2+data_euler[2,1]+data_euler[2,-1])*0.1,data_euler[3,-1])


#Checking if below zero
print(np.where(data_euler[1:]<0))
print(np.where(data_RK45[1:]<0),len(data_RK45))

# Check monotone
for i in range(len(data_euler[3])-1):
    if data_euler[3, i]>data_euler[3, i+1]:
        print()
        print("R is not monotone", i)
        break
else:
    print("R is monotone")

for i in range(len(data_euler[1])-1):
    if data_euler[1,i]<data_euler[1,i+1]:
        print("S is not monotone", i)
        break
else:
    print("S is monotone")


# Check if max hældning
R_diff = np.diff(data_euler[3])
S_diff = np.diff(data_euler[1])
print(np.argmax(R_diff),np.argmin(S_diff),np.argmax(data_euler[2]))
