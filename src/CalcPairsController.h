#ifndef CALC_PAIRS_CONTROLLER_H
#define CALC_PAIRS_CONTROLLER_H

#include <stack>
#include <set>
#include <vector>
#include <string>

#include "DataContainer.h"

class CalcPairsController
{
private:
  const DataContainer *data;
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

  FILE *output;

  void calc_free_rows();
  void calc_free_cols();

  FILE* open_file(const std::string &file_name) const;
  void record_free_rows() const;
  void record_free_cols() const;
  void record_pair_count(FILE* stream, const std::vector<std::size_t> &pairs) const;

  void send_problem(const int rowCol, const std::size_t idx);
  void receive_completion();

  void send_start();

public:
  CalcPairsController(const DataContainer &_data,
                      const std::string &_scratch_dir);
  ~CalcPairsController();

  void work();
  void signal_workers_to_end();
  void wait_for_workers();
  bool workers_still_working();

  void combine_row_pair_files();
  void combine_col_pair_files();
};

#endif