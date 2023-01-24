#ifndef PRION_HPP
#define PRION_HPP

#include <deal.II/base/conditional_ostream.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/distributed/fully_distributed_tria.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/lac/solver_gmres.h>

#include <deal.II/fe/fe_simplex_p.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_values_extractors.h>
#include <deal.II/fe/mapping_fe.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/tria.h>

#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/trilinos_precondition.h>
#include <deal.II/lac/trilinos_sparse_matrix.h>

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/grid/grid_out.h>
#include <fstream>
#include <iostream>

using namespace dealii;

// Class representing the non-linear diffusion problem.
class HeatNonLinear {
public:
  // Physical dimension (1D, 2D, 3D)
  static constexpr unsigned int dim = 3;

  // Function for the mu_0 coefficient.
  class FunctionAlpha : public Function<dim> {
  public:
    virtual double
    value(const Point<dim> & /*p*/, const unsigned int /*component*/ = 0) const override {
      return 2.0;
    }
  };

  Tensor<2, dim>
  SpreadingTensor() {
    Tensor<2, dim> result;

      // duex / dx
      for (unsigned int i = 1; i < dim + 1; ++i) {
          for (unsigned int j = 1; j < dim + 1; ++j) {
            if (i != j)
              result[i][j] = 0.0002;
            else
              result[i][j] = 0.0004;
          }
      }

    return result;
  }

  // Function for initial conditions.
  class FunctionU0 : public Function<dim> {
  public:
    virtual double
    value(const Point<dim> &p, const unsigned int /*component*/ = 0) const override {
      if (p[0] == 0.0 && p[1] == 0.5 && p[2] == 0.5)
        return 0.1;
      return 0.0;
    }
  };

  // Constructor. We provide the final time, time step Delta t and theta method
  // parameter as constructor arguments.
  HeatNonLinear(const unsigned int &N_,
                const unsigned int &r_,
                const double       &T_,
                const double       &deltat_) :
    mpi_size(Utilities::MPI::n_mpi_processes(MPI_COMM_WORLD)),
    mpi_rank(Utilities::MPI::this_mpi_process(MPI_COMM_WORLD)),
    pcout(std::cout, mpi_rank == 0), T(T_), N(N_), r(r_), deltat(deltat_),
    mesh(MPI_COMM_WORLD) {}

  // Initialization.
  void
  setup();

  // Solve the problem.
  void
  solve();

protected:
  // Assemble the tangent problem.
  void
  assemble_system();

  // Solve the linear system associated to the tangent problem.
  void
  solve_linear_system();

  // Solve the problem for one time step using Newton's method.
  void
  solve_newton();

  // Output.
  void
  output(const unsigned int &time_step, const double &time) const;

  // MPI parallel. /////////////////////////////////////////////////////////////

  // Number of MPI processes.
  const unsigned int mpi_size;

  // This MPI process.
  const unsigned int mpi_rank;

  // Parallel output stream.
  ConditionalOStream pcout;

  // Problem definition. ///////////////////////////////////////////////////////

  // mu_0 coefficient.
  FunctionAlpha alpha;

  // Initial conditions.
  FunctionU0 u_0;

  // Current time.
  double time;

  // Final time.
  const double T;

  // Discretization. ///////////////////////////////////////////////////////////

  // Mesh refinement.
  const unsigned int N;

  // Polynomial degree.
  const unsigned int r;

  // Time step.
  const double deltat;

  // Mesh.
  parallel::fullydistributed::Triangulation<dim> mesh;

  // Finite element space.
  std::unique_ptr<FiniteElement<dim>> fe;

  // Quadrature formula.
  std::unique_ptr<Quadrature<dim>> quadrature;

  // DoF handler.
  DoFHandler<dim> dof_handler;

  // DoFs owned by current process.
  IndexSet locally_owned_dofs;

  // DoFs relevant to the current process (including ghost DoFs).
  IndexSet locally_relevant_dofs;

  // Jacobian matrix.
  TrilinosWrappers::SparseMatrix jacobian_matrix;

  // Residual vector.
  TrilinosWrappers::MPI::Vector residual_vector;

  // Increment of the solution between Newton iterations.
  TrilinosWrappers::MPI::Vector delta_owned;

  // System solution (without ghost elements).
  TrilinosWrappers::MPI::Vector solution_owned;

  // System solution (including ghost elements).
  TrilinosWrappers::MPI::Vector solution;

  // System solution at previous time step.
  TrilinosWrappers::MPI::Vector solution_old;
};

#endif