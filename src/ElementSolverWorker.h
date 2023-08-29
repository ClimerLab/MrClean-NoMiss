#ifndef ELEMENT_SOLVER_WORKER_H
#define ELEMENT_SOLVER_WORKER_H

#include <vector>
#include <string>
#include "DataContainer.h"
#include "Pairs.h"
#include "ElementIpSolver.h"

class ElementSolverWorker
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::string scratch_file;
  const std::size_t world_rank;
  const std::size_t LARGE_MATRIX;
  bool end_;

  std::size_t row_sum;
  std::size_t min_cols;
  std::vector<int> valid_row;
  std::vector<int> valid_col;

  std::size_t obj_value;
  std::vector<int> rows_to_keep;
  std::vector<int> cols_to_keep;

  std::vector<std::size_t> forced_one_rows;
  std::vector<std::size_t> forced_one_cols;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

  std::vector<std::pair<std::size_t, std::vector<std::size_t>>> row_pairs;
  std::vector<std::pair<std::size_t, std::vector<std::size_t>>> col_pairs;

  FILE* open_file_for_read(const std::string &file_name) const;
  void read_forced_one_rows();
  void read_forced_one_cols();
  void read_free_rows();
  void read_free_cols();

  void receive_problem();
  void send_back_solution();

  void clear_pairs();

public:
  ElementSolverWorker(const DataContainer &_data,
                      const std::string &_scratch_dir,
                      const std::size_t _LARGE_MATRIX);
  ~ElementSolverWorker();

  void work();
  bool end() const;
};
#endif