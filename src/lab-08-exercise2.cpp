#include "HeatNonLinear.hpp"

// Main function.
int
main(int argc, char *argv[])
{
  Utilities::MPI::MPI_InitFinalize mpi_init(argc, argv);

  const unsigned int N      = 200;
  const unsigned int degree = 1;

  const double T      = 20.0;
  const double deltat = 0.1;

  HeatNonLinear problem(N, degree, T, deltat);

  problem.setup();
  problem.solve();

  return 0;
}