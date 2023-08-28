#ifndef ELEMENT_IP_SOLVER_H
#define ELEMENT_IP_SOLVER_H

#include <vector>
#include <ilcplex/ilocplex.h>
#include <ilconcert/ilomodel.h>

#include "DataContainer.h"

class ElementIpSolver
{
private:
  const DataContainer *data;
  const std::vector<size_t> *forced_one_rows;
  const std::vector<size_t> *forced_one_cols;
  const std::vector<std::size_t> *free_rows;
  const std::vector<std::size_t> *free_cols;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::size_t row_sum;
  const std::size_t min_cols;
  const std::size_t LARGE_MATRIX;
  const double TOL;

  std::vector<double> r_var;
  std::vector<double> c_var;
  std::size_t obj_value;

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
  bool is_large_matrix() const;

public:
  ElementIpSolver(const DataContainer &_data,
                  const std::size_t _row_sum,
                  const std::size_t _min_cols,
                  const std::vector<size_t> &_forced_one_rows,
                  const std::vector<size_t> &_forced_one_cols,
                  const std::vector<std::size_t> &_free_rows,
                  const std::vector<std::size_t> &_free_cols,
                  const std::size_t _LARGE_MATRIX,
                  const double _TOL = 0.00001);
  ~ElementIpSolver();

  void set_row_to_zero(const std::size_t row_idx);
  void set_col_to_zero(const std::size_t col_idx);
  void set_row_to_one(const std::size_t row_idx);
  void set_col_to_one(const std::size_t col_idx);
  void add_row_pairs_cut(const std::size_t row_idx, const std::vector<std::size_t> &pairs);
  void add_col_pairs_cut(const std::size_t col_idx, const std::vector<std::size_t> &pairs);
  void solve();

  std::size_t get_obj_value() const;
  std::size_t get_num_elements() const;
  std::vector<int> get_rows_to_keep() const;
  std::vector<int> get_cols_to_keep() const;
};

#endif