# Prion-disease
Implementation of a simulation of Prion spreading inside a human brain mesh using Gmsh, Deal.II Library and Paraview for the visualization of the results. To characterize the progression of misfolded proteins across the brain, the classical Fisher–Kolmogorov equation for population dynamics is used together with an anisotropic diffusion model and simulate misfolding across a sagittal section and across the entire brain.

Our work takes its references from the article "Weickenmeier et al. - 2019 - A physics-based model explains the prion-like features of neurodegeneration in Alzheimers disease" reported here.

src_with_integrals contains an implementation of the calculation of the integral of the variacle c(x,t) across the volume of the brain. A .csv file is created to plot the results on a graph using a Phyton script. It is put in another folder not to change the results about timing and performances of the standard implementation.

We made it run both locally and on Politecnico di Milano's supercluster.

Please read PrionDeseade.pdf (it's without animations).

Here is a preview:

![image](https://github.com/valentina-moretti/Prion-disease/assets/100996597/de0bef68-2b09-49fd-bd8d-674e66bd4400)

![image](https://github.com/valentina-moretti/Prion-disease/assets/100996597/8fbb6f06-5716-458b-8792-3414255c353c)

![image](https://github.com/valentina-moretti/Prion-disease/assets/100996597/64ce56cf-355d-4565-87bf-92934b5bc04c)


![PrionDisease med](https://github.com/valentina-moretti/Prion-disease/assets/100996597/4ffd1ad0-fede-4bc6-a874-313d951900f1)
![PrionDisease2](https://github.com/valentina-moretti/Prion-disease/assets/100996597/0e0c4b52-5995-453a-8ac4-2c765c02de7c)
![PrionDisease3](https://github.com/valentina-moretti/Prion-disease/assets/100996597/27daba31-fb42-419b-950e-d1b0370ef1bc)


MPI partitioning with 10 cores:

![image](https://github.com/valentina-moretti/Prion-disease/assets/100996597/ecc26d22-2bf7-4d92-b80c-1d5b25d14834)
