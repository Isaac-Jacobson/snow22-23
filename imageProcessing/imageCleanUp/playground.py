import numpy as np

a = np.array([[1, 2, 3], [4, 5, 6]])
b = np.zeros((len(a), 1))
c = np.zeros((1, 5))
d = np.concatenate((np.zeros((len(a), 1)), a, np.zeros((len(a), 1))), axis = 1)
print(d)
print(np.zeros((1, len(d[0]))))
e = np.concatenate((np.zeros((1, len(d[0]))), d, np.zeros((1, len(d[0])))), axis = 0)


print(e)