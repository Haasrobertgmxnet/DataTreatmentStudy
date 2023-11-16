# Author: Jake Vanderplas <jakevdp@cs.washington.edu>
#
import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm

from sklearn.neighbors import KernelDensity

f = open("C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample1.txt", "r")
X = np.array([float(x) for x in f.read().split("\n")])

X= X.reshape(144, 1)

f = open("C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample2.txt", "r")
W = np.array([[float(x) for x in y.split("\t")] for y in (f.read().split("\n"))[2:]])

W= W.reshape(512, 8)
print(W)

print(W[:,0])
X= X.reshape(144, 1)

kde = KernelDensity(kernel="gaussian", bandwidth=40.0).fit(X)
log_dens = kde.score_samples(W[:,0].reshape(512,1))
np.exp(log_dens).tofile("C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample3.txt", sep="\n")

kde = KernelDensity(kernel="epanechnikov", bandwidth=40.0).fit(X)
log_dens = kde.score_samples(W[:,0].reshape(512,1))
np.exp(log_dens).tofile("C:\\Users\\haasr\\source\\repos\\DataTreatmentStudy\\ProbFuncTest\\data\\sample4.txt", sep="\n")

