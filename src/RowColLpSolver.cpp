#include "RowColLpSolver.h"

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
RowColLpSolver::RowColLpSolver(const BinContainer &_data,
                               const double _TOL) : data(&_data),
                                                    num_rows(data->get_num_data_rows()),
                                                    num_cols(data->get_num_data_cols()),
                                                    TOL(_TOL),
                                                    r_var(num_rows),
                                                    c_var(num_cols),
                                                    obj_value(0.0),
                                                    env(IloEnv()),
                                                    cplex(IloCplex(env)),
                                                    model(IloModel(env)),
                                                    r(IloNumVarArray(env, num_rows, 0, 1)),
                                                    c(IloNumVarArray(env, num_cols, 0, 1)),
                                                    r_copy(IloNumArray(env, num_rows)),
                                                    c_copy(IloNumArray(env, num_cols)),
                                                    obj(IloExpr(env))
{
  r.setNames("r");
  c.setNames("c");
  build_model();
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
RowColLpSolver::~RowColLpSolver() {}

//------------------------------------------------------------------------------
// Builds the CPLEX model based on the BinContainer object.
//------------------------------------------------------------------------------
void RowColLpSolver::build_model() {
  for (std::size_t i = 0; i < num_rows; ++i) {
    obj += static_cast<IloNum>(data->get_num_valid_in_row(i)) * r[i];
  }
  for (std::size_t j = 0; j < num_cols; ++j) {
    obj += static_cast<IloNum>(data->get_num_valid_in_col(j)) * c[j];
  }
  model.add(IloMaximize(env, obj, "Objective"));

  for (std::size_t i = 0; i < num_rows; ++i) {

    for (std::size_t j = 0; j < num_cols; ++j) {
      if (data->is_data_na(i,j)) {
        model.add(r[i] + c[j] <= 1);
      }
    }
    // std::size_t num_excluded = 0;
    // IloExpr col_sum_expr(env);
    // for (std::size_t j = 0; j < num_cols; ++j) {
    //   if (data->is_data_na(i,j)) {
    //     col_sum_expr += c[j];
    //     ++num_excluded;
    //   }
    // }
    // if (num_excluded > 0) {
    //   model.add(static_cast<IloNum>(num_excluded) * r[i] + col_sum_expr <= static_cast<IloNum>(num_excluded));
    // }
    // col_sum_expr.end();
  }
}

//------------------------------------------------------------------------------
// Rounds values close to 1 or 0 to 1 or 0 respectively. Values must be within
// 'TOL' of 1 or 0 to be rounded. Row decision variables and column decision 
// variables are both rounded.
//------------------------------------------------------------------------------
void RowColLpSolver::round_extreme_values() {
  for (std::size_t i = 0; i < r_var.size(); ++i) {
    if (r_var[i] > 1.0 - TOL) {
      r_var[i] = 1.0;
    } else if (r_var[i] < TOL) {
      r_var[i] = 0.0;
    }
  }
  for (std::size_t i = 0; i < c_var.size(); ++i) {
    if (c_var[i] > 1.0 - TOL) {
      c_var[i] = 1.0;
    } else if (c_var[i] < TOL) {
      c_var[i] = 0.0;
    }
  }
}

//------------------------------------------------------------------------------
// Calls CPLEX solver and rounds decision variables if a valid solution is
// found.
//------------------------------------------------------------------------------
void RowColLpSolver::solve() {
  cplex.extract(model);
  
  cplex.setParam(IloCplex::Param::RandomSeed, 0);
  cplex.setParam(IloCplex::Param::Threads, 1);
  cplex.setParam(IloCplex::Param::RootAlgorithm, IloCplex::Algorithm::Barrier);
  cplex.setOut(env.getNullStream());

  obj_value = 0.0;
  try {
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
      printf("Infeasible Solution\n");
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
      obj_value = cplex.getObjValue(); // Save objective value
      
      // Copy decision variables
      cplex.getValues(r, r_copy);
      cplex.getValues(c, c_copy);      
      for (std::size_t i = 0; i < r_var.size(); ++i) {
        r_var[i] = r_copy[i];
      }
      for (std::size_t i = 0; i < c_var.size(); ++i) {
        c_var[i] = c_copy[i];
      }
      
      round_extreme_values();
    }
  } catch (IloException& e) {
    std::cerr << "Concert exception caught: " << e << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
  }
  env.end();
}

//------------------------------------------------------------------------------
// Returns the objective value from CPLEX. If CPLEX has not been called, or did
// not find a solution, then 0 is returned.
//------------------------------------------------------------------------------
double RowColLpSolver::get_obj_value() const {
  return obj_value;
}

//------------------------------------------------------------------------------
// Returns the rows to keep as a boolean vector. In order for a row to be kept
// the corresponding decision variable must equal 1.0.
//------------------------------------------------------------------------------
std::vector<bool> RowColLpSolver::get_rows_to_keep() const {
  std::vector<bool> rows_to_keep(num_rows, false);

  for (std::size_t i = 0; i < num_rows; ++i) {
    if (r_var[i] == 1.0) {
      rows_to_keep[i] = true;
    }
  }

  return rows_to_keep;
}

//------------------------------------------------------------------------------
// Returns the colums to keep as a boolean vector. In order for a column to be
// kept the corresponding decision variable must equal 1.0.
//------------------------------------------------------------------------------
std::vector<bool> RowColLpSolver::get_cols_to_keep() const {
  std::vector<bool> cols_to_keep(num_cols, false);

  for (std::size_t j = 0; j < num_cols; ++j) {
    if (c_var[j] == 1.0) {
      cols_to_keep[j] = true;
    }
  }

  return cols_to_keep;
}

//------------------------------------------------------------------------------
// Return the number of rows keep. In order for a row to be kept the
// corresponding decision variable must equal 1.0.
//------------------------------------------------------------------------------
std::size_t RowColLpSolver::get_num_rows_to_keep() const {
  std::size_t count = 0;
  
  for (std::size_t i = 0; i < num_rows; ++i) {
    if (r_var[i] == 1.0) {
      ++count;
    }
  }

  return count;
}

//------------------------------------------------------------------------------
// Return the number of columns keep. In order for a column to be kept the
// corresponding decision variable must equal 1.0.
//------------------------------------------------------------------------------
std::size_t RowColLpSolver::get_num_cols_to_keep() const {
  std::size_t count = 0;
  
  for (std::size_t j = 0; j < num_cols; ++j) {
    // if (variables[num_rows + j] == 1.0) {
    if (c_var[j] == 1.0) {
      ++count;
    }
  }
  
  return count;
}

//------------------------------------------------------------------------------
// Returns the number of valid elements kept, by mulitplying the number of rows
// kept by the number of columns kept.
//------------------------------------------------------------------------------
std::size_t RowColLpSolver::get_num_elements_to_keep() const {
  return get_num_rows_to_keep() * get_num_cols_to_keep();
}