import numpy as np
import matplotlib.pyplot as plt
plt.close('all')

file = open('HPPC_atom_stats.txt')
lines = file.readlines()

new_lines = []
for l in lines:
    new_lines.append(l.split())

new_lines.pop(49)

tags = ['2_mol', '4_mol', '16_mol', '128_mol']
mol = [2, 4, 16, 128]

data = {k: [] for k in tags}

N_lines = 10
for i in range(N_lines):
    data[tags[0]].append([new_lines[17+i][j] for j in range(7)])
    data[tags[1]].append([new_lines[3+i][j]  for j in range(7)])
    data[tags[2]].append([new_lines[31+i][j] for j in range(7)])
    data[tags[3]].append([new_lines[45+i][j] for j in range(7)])

data[tags[2]][1], data[tags[2]][2] = data[tags[2]][2], data[tags[2]][1]

labels = ['UpdateNonBondedForces','UpdateBondedForces','UpdateAngleForces','Evolve','Other Functions']

plt.figure()
for i in range(5):
    plt.plot(mol, [eval(data[k][i][0]) for k in tags], label=labels[i])
plt.legend()
plt.grid()
plt.xscale('log')
plt.xlabel('Number of Molecules')
plt.ylabel('Runtime Percentage')
plt.savefig('Vectorized_performance.eps')