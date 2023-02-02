# Prion-disease
Implementation of a simulation of Prion spreading inside a human brain mesh using Gmsh, Deal.II Library and Paraview for the visualization of the results. To characterize the progression of misfolded proteins across the brain, the classical Fisher–Kolmogorov equation for population dynamics is used together with an anisotropic diffusion model and simulate misfolding across a sagittal section and across the entire brain.

Our work takes its references from the article "Weickenmeier et al. - 2019 - A physics-based model explains the prion-like features of neurodegeneration in Alzheimers disease" reported here.

src_with_integrals contains an implementation of the calculation of the integral of the variacle c(x,t) across the volume of the brain. A .csv file is created to plot the results on a graph using a Phyton script. It is put in another folder not to change the results about timing and performances of the standard implementation.
