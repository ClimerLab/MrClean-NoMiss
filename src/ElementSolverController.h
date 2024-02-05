#ifndef ELEMENT_SOLVER_CONTROLLER_H
#define ELEMENT_SOLVER_CONTROLLER_H

#include <stack>
#include <set>
#include <vector>
#include <string>

#include "BinContainer.h"
#include "Pairs.h"

class ElementSolverController
{
private:
  const BinContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::string scratch_dir;

  const std::size_t world_size;

  std::stack<int> available_workers;
  std::set<int> unavailable_workers;

  std::vector<std::size_t> forced_one_rows;
  std::vector<std::size_t> forced_one_cols;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

  Pairs row_pairs;
  Pairs col_pairs;

  std::vector<int> valid_row;
  std::vector<int> valid_col;

  std::size_t best_num_elements;
  std::vector<int> best_rows_to_keep;
  std::vector<int> best_cols_to_keep;

  FILE* open_file_for_read(const std::string &file_name) const;
  void read_forced_one_rows();
  void read_forced_one_cols();
  void read_free_rows();
  void read_free_cols();

  void send_problem(const std::size_t row_sum,
                    const std::size_t min_cols);

  void receive_completion();  

public:
  ElementSolverController(const BinContainer &_data,
                          const std::string &_scratch_dir,
                          const std::string &incumbent_file);
  ~ElementSolverController();

  void work();
  void signal_workers_to_end();
  void wait_for_workers();
  bool workers_still_working();

  std::vector<int> get_rows_to_keep() const;
  std::vector<int> get_cols_to_keep() const;
  std::size_t get_num_rows_to_keep() const;
  std::size_t get_num_cols_to_keep() const;
};

#endif