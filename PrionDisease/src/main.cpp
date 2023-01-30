#include "Prion.hpp"

// Main function.
int
main(int argc, char *argv[]) {
  Utilities::MPI::MPI_InitFinalize mpi_init(argc, argv);

  const unsigned int N      = 19;
  const unsigned int degree = 1;

  const double T      = 10.0;
  const double deltat = 0.1;

  HeatNonLinear problem(N, degree, T, deltat);

  problem.setup();
  problem.solve();

  std::ofstream file_out("Integral.csv");

  if(Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)==0){
    //problem.timer_output.enter_subsection("Writing csv");
    for(size_t i = 0; i< problem.integral.size(); ++i){
      file_out << i << "," << problem.integral[i] << std::endl;
    }
    //problem.timer_output.leave_subsection();
  }
  return 0;
}