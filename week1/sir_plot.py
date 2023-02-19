import numpy as np
import matplotlib.pyplot as plt
plt.close('all')

data = np.loadtxt('SIR_data.txt', skiprows = 1)
t = data[:,0]
S = data[:,1]
I = data[:,2]
R = data[:,3]

plt.plot(t, S, label='Susceptible')
plt.plot(t, I, label='Infected')
plt.plot(t, R, label='Recovered')
plt.plot(t, S+I+R, label='Sum of parameters')
plt.legend()