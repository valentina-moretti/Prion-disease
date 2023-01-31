#include "Prion.hpp"

// Main function.
int
main(int argc, char *argv[]) {
  Utilities::MPI::MPI_InitFinalize mpi_init(argc, argv);

  const unsigned int N      = 99;
  const unsigned int degree = 1;

  const double T      = 10.0;
  const double deltat = 0.1;

  HeatNonLinear problem(N, degree, T, deltat);

  problem.setup();
  problem.solve();

  

  std::ofstream file_out("Integral.csv");
  int mpi_rank = Utilities::MPI::this_mpi_process(MPI_COMM_WORLD);
  problem.timer_output.enter_subsection("Writing csv");
  if(mpi_rank == 0){
    file_out << "timestep,integral" << std::endl;
    for(size_t i = 0; i< problem.integral.size(); ++i){
      file_out << i << "," << problem.integral[i] << std::endl;
    }
  }
  else{
    std::cout<< "i'm rank "<< mpi_rank<< std::endl;
  } 
  problem.timer_output.leave_subsection();

  return 0;
}