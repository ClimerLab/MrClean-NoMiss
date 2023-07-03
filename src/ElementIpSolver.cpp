#include "ElementIpSolver.h"
#include <assert.h>

ElementIpSolver::ElementIpSolver(const DataContainer &_data,
                                 const std::size_t _row_sum,
                                 const std::size_t _min_cols,
                                 const std::vector<size_t> &_forced_one_rows,
                                 const std::vector<size_t> &_forced_one_cols,
                                 const std::vector<std::size_t> &_free_rows,
                                 const std::vector<std::size_t> &_free_cols,
                                 const double _TOL) : data(&_data),
                                                      forced_one_rows(&_forced_one_rows),
                                                      forced_one_cols(&_forced_one_cols),
                                                      free_rows(&_free_rows),
                                                      free_cols(&_free_cols),
                                                      num_rows(free_rows->size()),
                                                      num_cols(free_cols->size()),
                                                      row_sum(_row_sum),
                                                      min_cols(_min_cols),
                                                      TOL(_TOL),
                                                      r_var(num_rows),
                                                      c_var(num_cols),
                                                      obj_value(0),
                                                      env(IloEnv()),
                                                      cplex(IloCplex(env)),
                                                      model(IloModel(env)),
                                                      r(IloNumVarArray(env, num_rows, 0, 1, ILOINT)),
                                                      c(IloNumVarArray(env, num_cols, 0, 1, ILOINT)),
                                                      r_copy(IloNumArray(env, num_rows)),
                                                      c_copy(IloNumArray(env, num_cols)),
                                                      obj(IloExpr(env)) {
  r.setNames("r");
  c.setNames("c");
  build_model();
}

ElementIpSolver::~ElementIpSolver() {}

void ElementIpSolver::build_model() {
  // Add the objective value
  for (std::size_t j = 0; j < num_cols; ++j) {
    obj += c[j];
  }
  obj += static_cast<IloNum>(forced_one_cols->size());
  model.add(IloMaximize(env, obj, "Objective"));

  // Add the constraint for the row_sum
  IloExpr row_sum_expr(env);
  for (std::size_t i = 0; i < num_rows; ++i) {
    row_sum_expr += r[i];
  }
  model.add(row_sum_expr == static_cast<IloNum>(row_sum - forced_one_rows->size()));

  // Add constraints for missing data
  for (std::size_t local_i = 0; local_i < num_rows; ++local_i) {
    const std::size_t i = free_rows->at(local_i);

    std::vector<std::size_t> cols_to_exclude;
    for (std::size_t local_j = 0; local_j < num_cols; ++local_j) {
      const std::size_t j = free_cols->at(local_j);

      if (data->is_data_na(i,j)) {
        // cols_to_exclude.push_back(local_j);
        model.add(r[local_i] + c[local_j] <= 1);
      }
    }

    // if (!cols_to_exclude.empty()) {
    //   IloExpr col_sum_expr(env);
    //   for (auto col : cols_to_exclude) {
    //     col_sum_expr += c[col];
    //   }
    //   model.add(static_cast<IloNum>(cols_to_exclude.size()) * r[local_i] + col_sum_expr <= static_cast<IloNum>(cols_to_exclude.size()));
    //   col_sum_expr.end();
    // }
  }
}

void ElementIpSolver::round_extreme_values() {
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

void ElementIpSolver::set_row_to_zero(const std::size_t row_idx) {
  assert(row_idx < num_rows);
  model.add(r[row_idx] == 0);
}

void ElementIpSolver::set_col_to_zero(const std::size_t col_idx) {
  assert(col_idx < num_cols);
  model.add(c[col_idx] == 0);
}

void ElementIpSolver::set_row_to_one(const std::size_t row_idx) {
  assert(row_idx < num_rows);
  model.add(r[row_idx] == 1);
}

void ElementIpSolver::set_col_to_one(const std::size_t col_idx) {
  assert(col_idx < num_cols);
  model.add(c[col_idx] == 1);
}

void ElementIpSolver::add_row_pairs_cut(const std::size_t row_idx, const std::vector<std::size_t> &pairs) {
  assert(row_idx < num_rows);
  assert(pairs.size() > 0);

  for (auto p : pairs) {
    model.add(r[row_idx] + r[p] <= 1);
  }
  // IloExpr row_sum_epxr(env);
  // for (auto p : pairs) {
  //   row_sum_epxr += r[p];
  // }
  // model.add(static_cast<IloNum>(pairs.size()) * r[row_idx] + row_sum_epxr <= static_cast<IloNum>(pairs.size()));
  // row_sum_epxr.end();
}

void ElementIpSolver::add_col_pairs_cut(const std::size_t col_idx, const std::vector<std::size_t> &pairs) {
  assert(col_idx < num_cols);
  assert(pairs.size() > 0);

  for (auto p : pairs) {
    model.add(c[col_idx] + c[p] <= 1);
  }
  // IloExpr col_sum_epxr(env);
  // for (auto p : pairs) {
  //   col_sum_epxr += r[p];
  // }
  // model.add(static_cast<IloNum>(pairs.size()) * c[col_idx] + col_sum_epxr <= static_cast<IloNum>(pairs.size()));
  // col_sum_epxr.end();
}

void ElementIpSolver::solve() {
  cplex.extract(model);
// cplex.exportModel("element.lp");
  if (min_cols > 0) {
    cplex.setParam(IloCplex::Param::MIP::Tolerances::LowerCutoff, min_cols);
  }
  
  obj_value = 0;

  cplex.setParam(IloCplex::Param::RandomSeed, 0);
  cplex.setParam(IloCplex::Param::Threads, 1);
  cplex.setOut(env.getNullStream());

  try {
    cplex.solve();

    if (cplex.getStatus() == IloAlgorithm::Infeasible) {
      // printf("Infeasible Solution\n");
      obj_value = 0;
    } else if (cplex.getStatus() == IloAlgorithm::Optimal) {
      obj_value = static_cast<std::size_t>(std::round(cplex.getObjValue()));
      cplex.getValues(r, r_copy);
      cplex.getValues(c, c_copy);

      for (std::size_t i = 0; i < r_var.size(); ++i) {
        r_var[i] = r_copy[i];
      }
      for (std::size_t j = 0; j < c_var.size(); ++j) {
        c_var[j] = c_copy[j];
      }

      round_extreme_values();
    }
    env.end();

  } catch (IloException& e) {
    std::cerr << "Concert exception caught: " << e << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
  }
}

std::size_t ElementIpSolver::get_obj_value() const {
  return obj_value;
}

std::size_t ElementIpSolver::get_num_elements() const {
  return obj_value * row_sum;
}

std::vector<int> ElementIpSolver::get_rows_to_keep() const {
  std::vector<int> rows_to_keep(num_rows + forced_one_rows->size(), 0);

  for (std::size_t i = 0; i < forced_one_rows->size(); ++i) {
    rows_to_keep[forced_one_rows->at(i)] = 1;
  }
  for (std::size_t i = 0; i < num_rows; ++i) {
    if (r_var[i] == 1.0) {
      rows_to_keep[free_rows->at(i)] = 1;
    }
  }

  return rows_to_keep;
}

std::vector<int> ElementIpSolver::get_cols_to_keep() const {
  std::vector<int> cols_to_keep(num_cols + forced_one_cols->size(), 0);

  for (std::size_t j = 0; j < forced_one_cols->size(); ++j) {
    cols_to_keep[forced_one_cols->at(j)] = 1;
  }
  for (std::size_t j = 0; j < num_cols; ++j) {
    if (c_var[j] == 1.0) {
      cols_to_keep[free_cols->at(j)] = 1;
    }
  }

  return cols_to_keep;
}