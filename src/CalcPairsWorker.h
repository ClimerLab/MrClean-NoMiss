#ifndef CALC_PAIRS_WORKER_H
#define CALC_PAIRS_WORKER_H

#include <vector>
#include "DataContainer.h"

class CalcPairsWorker
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::size_t world_rank;

  std::vector<std::size_t> forced_one_rows;
  std::vector<std::size_t> forced_one_cols;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

  int row_col;
  std::size_t idx;
  std::vector<std::size_t> count;

  bool end_;

  void calc_free_rows();
  void calc_free_cols();

  void receive_problem();
  void send_back_solution();

public:
  CalcPairsWorker(const DataContainer &_data);
  ~CalcPairsWorker();

  void work();
  bool end() const;
};
#endif