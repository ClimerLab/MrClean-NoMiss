#ifndef CALC_PAIRS_WORKER_H
#define CALC_PAIRS_WORKER_H

#include <vector>
#include "BinContainer.h"

class CalcPairsWorker
{
private:
  const BinContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::string scratch_dir;
  const std::size_t world_rank;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

  int row_col;
  std::size_t idx;
  std::vector<std::size_t> count;

  bool end_;

  FILE* open_file_for_read(const std::string &file_name) const;
  void read_free_rows();
  void read_free_cols();

  void send_completion();
  void receive_start();

public:
  CalcPairsWorker(const BinContainer &_data, 
                  const std::string &_scratch_dir);
  ~CalcPairsWorker();

  void work();
  bool end() const;
};
#endif