#include "Prion.hpp"

void
HeatNonLinear::setup() {
  // Create the mesh.
  {
    std::cout << "Initializing the mesh" << std::endl;
    GridGenerator::subdivided_hyper_cube(mesh, N + 1, 0.0, 1.0, true);
    std::cout << "  Number of elements = " << mesh.n_active_cells() << std::endl;

    // Write the mesh to file.
    const std::string mesh_file_name = "mesh-" + std::to_string(N + 1) + ".vtk";
    GridOut           grid_out;
    std::ofstream     grid_out_file(mesh_file_name);
    grid_out.write_vtk(mesh, grid_out_file);
    std::cout << "  Mesh saved to " << mesh_file_name << std::endl;
  }

  std::cout << "-----------------------------------------------" << std::endl;

  // Initialize the finite element space.
  {
    std::cout << "Initializing the finite element space" << std::endl;

    fe = std::make_unique<FE_Q<dim>>(r);

    std::cout << "  Degree                     = " << fe->degree << std::endl;
    std::cout << "  DoFs per cell              = " << fe->dofs_per_cell << std::endl;

    quadrature = std::make_unique<QGauss<dim>>(r + 1);

    std::cout << "  Quadrature points per cell = " << quadrature->size() << std::endl;
  }

  std::cout << "-----------------------------------------------" << std::endl;

  // Initialize the DoF handler.
  {
    std::cout << "Initializing the DoF handler" << std::endl;

    dof_handler.reinit(mesh);
    dof_handler.distribute_dofs(*fe);

    std::cout << "  Number of DoFs = " << dof_handler.n_dofs() << std::endl;
  }

  std::cout << "-----------------------------------------------" << std::endl;

  // Initialize the linear system.
  {
    std::cout << "Initializing the linear system" << std::endl;

    std::cout << "  Initializing the sparsity pattern" << std::endl;
    DynamicSparsityPattern dsp(dof_handler.n_dofs());
    DoFTools::make_sparsity_pattern(dof_handler, dsp);
    sparsity_pattern.copy_from(dsp);

    std::cout << "  Initializing the matrices" << std::endl;
    jacobian_matrix.reinit(sparsity_pattern);

    std::cout << "  Initializing the system right-hand side" << std::endl;
    residual_vector.reinit(dof_handler.n_dofs());
    std::cout << "  Initializing the solution vector" << std::endl;
    solution_old.reinit(dof_handler.n_dofs());
    delta.reinit(dof_handler.n_dofs());

    solution.reinit(dof_handler.n_dofs());
    solution_old = solution;
  }
}

void
HeatNonLinear::assemble_system() {
  const unsigned int dofs_per_cell = fe->dofs_per_cell;
  const unsigned int n_q           = quadrature->size();

  FEValues<dim> fe_values(*fe,
                          *quadrature,
                          update_values | update_gradients | update_quadrature_points |
                            update_JxW_values);

  FullMatrix<double> cell_matrix(dofs_per_cell, dofs_per_cell);
  Vector<double>     cell_residual(dofs_per_cell);

  std::vector<types::global_dof_index> dof_indices(dofs_per_cell);

  jacobian_matrix = 0.0;
  residual_vector = 0.0;

  // Value and gradient of the solution on current cell.
  std::vector<double>         solution_loc(n_q);
  std::vector<Tensor<1, dim>> solution_gradient_loc(n_q);

  // Value of the solution at previous timestep (un) on current cell.
  std::vector<double> solution_old_loc(n_q);

    for (const auto &cell : dof_handler.active_cell_iterators()) {
      fe_values.reinit(cell);

      cell_matrix   = 0.0;
      cell_residual = 0.0;

      fe_values.get_function_values(solution, solution_loc);             // u n+1
      fe_values.get_function_gradients(solution, solution_gradient_loc); // grad u n+1
      fe_values.get_function_values(solution_old, solution_old_loc);     // u n

        for (unsigned int q = 0; q < n_q; ++q) {
          // Evaluate coefficients on this quadrature node.
          const double alpha_loc = alpha.value(fe_values.quadrature_point(q));

            for (unsigned int i = 0; i < dofs_per_cell; ++i) {
                for (unsigned int j = 0; j < dofs_per_cell; ++j) {
                  // ------------------------------------------- (A.1)
                  // ------------------------------------------- // Mass matrix.
                  cell_matrix(i, j) += fe_values.shape_value(i, q) *
                                       fe_values.shape_value(j, q) / deltat *
                                       fe_values.JxW(q);

                  // ------------------------------------------- (A.2)
                  // ------------------------------------------- // Non-linear stiffness
                  // matrix, first term.
                  cell_matrix(i, j) += fe_values.shape_grad(i, q) * D *
                                       fe_values.shape_grad(j, q) * fe_values.JxW(q);

                  // ------------------------------------------- (A.3)
                  // ------------------------------------------- // Non-linear stiffness
                  // matrix, second term.
                  cell_matrix(i, j) -= fe_values.shape_value(i, q) * alpha_loc *
                                       (1 - 2 * solution_loc[q]) *
                                       fe_values.shape_value(j, q) * fe_values.JxW(q);
                }

              // Assemble the residual vector (with changed sign).

              // ------------------------------------------- (R.1)
              // ------------------------------------------- // Time derivative term.
              cell_residual(i) -= (solution_loc[q] - solution_old_loc[q]) / deltat *
                                  fe_values.shape_value(i, q) * fe_values.JxW(q);

              // ------------------------------------------- (R.2)
              // ------------------------------------------- //
              cell_residual(i) -= fe_values.shape_grad(i, q) * D *
                                  solution_gradient_loc[q] * fe_values.JxW(q);

              // ------------------------------------------- (R.3)
              // ------------------------------------------- // Diffusion term.
              cell_residual(i) += (alpha_loc * solution_loc[q] * (1 - solution_loc[q])) *
                                  fe_values.shape_value(i, q) * fe_values.JxW(q);
            }
        }

      cell->get_dof_indices(dof_indices);

      jacobian_matrix.add(dof_indices, cell_matrix);
      residual_vector.add(dof_indices, cell_residual);
    }

  // We apply Dirichlet boundary conditions.
  // The linear system solution is delta, which is the difference between
  // u_{n+1}^{(k+1)} and u_{n+1}^{(k)}. Both must satisfy the same Dirichlet
  // boundary conditions: therefore, on the boundary, delta = u_{n+1}^{(k+1)} -
  // u_{n+1}^{(k+1)} = 0. We impose homogeneous Dirichlet BCs.
  // {
  //   std::map<types::global_dof_index, double> boundary_values;

  //   std::map<types::boundary_id, const Function<dim> *> boundary_functions;
  //   Functions::ZeroFunction<dim>                        zero_function;

  //   for (unsigned int i = 0; i < 2; ++i)
  //     boundary_functions[i] = &zero_function;

  //   VectorTools::interpolate_boundary_values(dof_handler,
  //                                            boundary_functions,
  //                                            boundary_values);

  //   MatrixTools::apply_boundary_values(
  //     boundary_values, jacobian_matrix, delta, residual_vector, false);
  // }
}

// TODO CHOOSE THE BETTER PRECONDITIONER
void
HeatNonLinear::solve_linear_system() {
  SolverControl solver_control(1000, 1e-6 * residual_vector.l2_norm());

  SolverCG<Vector<double>> solver(solver_control);
  // SolverGMRES<Vector<double>> solver(solver_control);
  PreconditionSSOR preconditioner;
  preconditioner.initialize(jacobian_matrix,
                            PreconditionSOR<SparseMatrix<double>>::AdditionalData(1.0));

  solver.solve(jacobian_matrix, delta, residual_vector, preconditioner);
  std::cout << "  " << solver_control.last_step() << " CG iterations" << std::endl;
  // std::cout << "  " << solver_control.last_step() << " GMRES iterations" << std::endl;
}

void
HeatNonLinear::solve_newton() {
  const unsigned int n_max_iters        = 1000;
  const double       residual_tolerance = 1e-6;

  unsigned int n_iter        = 0;
  double       residual_norm = residual_tolerance + 1;

  // We apply the boundary conditions to the initial guess (which is stored in
  // solution_owned and solution).
  {
    //?????
  }

    while (n_iter < n_max_iters && residual_norm > residual_tolerance) {
      assemble_system();
      residual_norm = residual_vector.l2_norm();

      std::cout << "  Newton iteration " << n_iter << "/" << n_max_iters
                << " - ||r|| = " << std::scientific << std::setprecision(6)
                << residual_norm << std::flush;

        // We actually solve the system only if the residual is larger than the
        // tolerance.
        if (residual_norm > residual_tolerance) {
          solve_linear_system();

          solution += delta;
        } else {
          std::cout << " < tolerance" << std::endl;
        }

      ++n_iter;
    }
}

void
HeatNonLinear::output(const unsigned int &time_step, const double &time) const {
  UNUSED(time);
  DataOut<dim> data_out;
  scratch      data_out.add_data_vector(dof_handler, solution, "u");

  data_out.build_patches();

  std::string output_file_name = std::to_string(time_step);

  // Pad with zeros.
  output_file_name = "/scratch/hpc/par8/output-" +
                     std::string(4 - output_file_name.size(), '0') + output_file_name +
                     ".vtk";

  std::ofstream output_file(output_file_name);
  data_out.write_vtk(output_file);
}

void
HeatNonLinear::solve() {
  std::cout << "===============================================" << std::endl;

  time = 0.0;

  // Apply the initial condition.
  {
    std::cout << "Applying the initial condition" << std::endl;

    VectorTools::interpolate(dof_handler, u_0, solution);

    // Output the initial solution.
    output(0, 0.0);
    std::cout << "-----------------------------------------------" << std::endl;
  }

  unsigned int time_step = 0;

    while (time < T - 0.5 * deltat) {
      time += deltat;
      ++time_step;

      // Store the old solution, so that it is available for assembly.
      solution_old = solution;

      std::cout << "n = " << std::setw(3) << time_step << ", t = " << std::setw(5)
                << std::fixed << time << std::endl;

      // At every time step, we invoke Newton's method to solve the non-linear
      // problem.
      solve_newton();

      output(time_step, time);

      std::cout << std::endl;
    }
}