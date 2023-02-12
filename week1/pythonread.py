import numpy as np
import matplotlib.pyplot as plt
S, I, R, _ = np.loadtxt('sir_out.txt', skiprows=1, unpack=True)
t = np.linspace(0, 500, len(S))
plt.plot(t, S)
plt.plot(t, I)
plt.plot(t, R)
plt.show()
