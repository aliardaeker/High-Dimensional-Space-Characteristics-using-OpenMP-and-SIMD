from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator, FormatStrFormatter
import numpy as np

fig = plt.figure()
ax = fig.gca(projection='3d')

# Make data.
X = np.arange(0.01, 1.01, 0.01)
Y = np.arange(16, 1, -1)
X, Y = np.meshgrid(X, Y)

file_name = 'data.txt'
with open(file_name) as f:
    data = f.readlines()

data = [x.strip() for x in data]
data = map(float, data)
zs = np.array(data)
Z = zs.reshape(X.shape)

# Plot the surface.
surf = ax.plot_surface(X, Y, Z, cmap=cm.coolwarm, linewidth=0, antialiased=False)

# Add a color bar which maps values to colors.
fig.colorbar(surf, shrink=0.5, aspect=5)

plt.show()