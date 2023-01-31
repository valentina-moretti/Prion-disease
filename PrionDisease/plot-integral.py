#!/usr/bin/env python

import matplotlib.pyplot as plt
import pandas as pd
import sys

convergence_data = pd.read_csv(sys.argv[1], sep = ",")

plt.rcParams.update({"font.size": 14})

plt.plot(convergence_data.timestep,
         convergence_data.integral,
         label = 'Integral of c')

plt.xscale("linear")
plt.yscale("linear")
plt.xlabel("timestep")
plt.ylabel("Integral of c")
plt.legend()

plt.savefig("integral.pdf")