#ifndef ELEMENT_SOLVER_WORKER_H
#define ELEMENT_SOLVER_WORKER_H

#include <vector>
#include "DataContainer.h"
#include "Pairs.h"

class ElementSolverWorker
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::size_t world_rank;
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

  Pairs row_pairs;
  Pairs col_pairs;

  FILE* open_file_for_read(const std::string &file_name) const;
  void read_forced_one_rows();
  void read_forced_one_cols();
  void read_free_rows();
  void read_free_cols();

  void receive_problem();
  void send_back_solution();

public:
  ElementSolverWorker(const DataContainer &_data);
  ~ElementSolverWorker();

  void work();
  bool end() const;
};
#endif