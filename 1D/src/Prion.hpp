#ifndef PRION_HPP
#define PRION_HPP

#include <deal.II/base/quadrature_lib.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_simplex_p.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_values_extractors.h>
#include <deal.II/fe/mapping_fe.h>

#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_out.h>
#include <deal.II/grid/tria.h>

#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/vector.h>

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/vector_tools.h>

#include <fstream>
#include <iostream>

using namespace dealii;

// Class representing the non-linear diffusion problem.
class HeatNonLinear {
public:
  // Physical dimension (1D, 2D, 3D)
  static constexpr unsigned int dim = 1;

  // Function for the mu_0 coefficient.
  class FunctionAlpha : public Function<dim> {
  public:
    virtual double
    value(const Point<dim> & /*p*/, const unsigned int /*component*/ = 0) const override {
      return 1.0;
    }
  };

  // Function for initial conditions.
  class FunctionU0 : public Function<dim> {
  public:
    virtual double
    value(const Point<dim> &p, const unsigned int /*component*/ = 0) const override {
      if (p[0] == 0.5 /* && p[1] == 0.5 && p[2] == 0.5*/)
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
    T(T_),
    N(N_), r(r_), deltat(deltat_) {}

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

  // Problem definition. ///////////////////////////////////////////////////////

  // mu_0 coefficient.
  FunctionAlpha alpha;

  // Initial conditions.
  FunctionU0 u_0;

  // Current time.
  double time;

  // Final time.
  const double T;

  // Diffusivity constant
  const double D = 0.0001;

  // Discretization. ///////////////////////////////////////////////////////////

  // Mesh refinement.
  const unsigned int N;

  // Polynomial degree.
  const unsigned int r;

  // Triangulation.
  Triangulation<dim> mesh;

  // Time step.
  const double deltat;

  // Finite element space.
  std::unique_ptr<FiniteElement<dim>> fe;

  // Quadrature formula.
  std::unique_ptr<Quadrature<dim>> quadrature;

  // DoF handler.
  DoFHandler<dim> dof_handler;

  // System solution.
  Vector<double> solution;
  // System solution.
  Vector<double> solution_old;

  // System solution.
  Vector<double> residual_vector;

  // System matrix.
  SparseMatrix<double> jacobian_matrix;

  // Sparsity pattern.
  SparsityPattern sparsity_pattern;

  // System solution.
  Vector<double> delta;
};

#endif