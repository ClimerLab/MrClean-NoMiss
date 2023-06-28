#ifndef ELEMENT_SOLVER_H
#define ELEMENT_SOLVER_H

#include <vector>
#include <string>
#include <ilcplex/ilocplex.h>
#include <ilconcert/ilomodel.h>

#include "DataContainer.h"
#include "Pairs.h"
class ElementSolver
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const double TOL;

  std::vector<std::size_t> forced_one_rows;
  std::vector<std::size_t> forced_one_cols;
  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;
  
  Pairs row_pairs;
  Pairs col_pairs;

  std::vector<bool> best_rows_to_keep;
  std::vector<bool> best_cols_to_keep;
  std::size_t best_num_elements;
  
  // std::vector<double> r_var;
  // std::vector<double> c_var;
  // double obj_value;

  // IloEnv env;
  // IloCplex cplex;
  // IloModel model;
  // IloNumVarArray r;
  // IloNumVarArray c;
  // IloNumArray r_copy;
  // IloNumArray c_copy;
  // IloExpr obj;

  FILE* open_file_for_read(const std::string &file_name) const;
  void read_forced_one_rows();
  void read_forced_one_cols();
  void read_free_rows();
  void read_free_cols();

  // void build_model();
  // void round_extreme_values();  

public:
  ElementSolver(const DataContainer &_data,
                const double _TOL = 0.00001);
  ~ElementSolver();

  void solve();

  std::vector<bool> get_rows_to_keep() const;
  std::vector<bool> get_cols_to_keep() const;
  std::size_t get_num_rows_to_keep() const;
  std::size_t get_num_cols_to_keep() const;
};

#endif