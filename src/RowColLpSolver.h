#ifndef ROW_COL_LP_SOLVER_H
#define ROW_COL_LP_SOLVER_H

#include <vector>
#include <ilcplex/ilocplex.h>
#include <ilconcert/ilomodel.h>

#include "BinContainer.h"

class RowColLpSolver
{
private:
  const BinContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const double TOL;

  std::vector<double> r_var;
  std::vector<double> c_var;
  double obj_value;

  IloEnv env;
  IloCplex cplex;
  IloModel model;
  IloNumVarArray r;
  IloNumVarArray c;
  IloNumArray r_copy;
  IloNumArray c_copy;
  IloExpr obj;

  void build_model();
  void round_extreme_values();

public:
  RowColLpSolver(const BinContainer &_data,
                 const double _TOL = 0.00001);
  ~RowColLpSolver();

  void solve();
  double get_obj_value() const;
  std::vector<bool> get_rows_to_keep() const;
  std::vector<bool> get_cols_to_keep() const;
  std::size_t get_num_rows_to_keep() const;
  std::size_t get_num_cols_to_keep() const;
  std::size_t get_num_elements_to_keep() const;
};



#endif
