#ifndef CALC_PAIRS_CORE_H
#define CALC_PAIRS_CORE_H

#include <vector>
#include <string>
#include "DataContainer.h"

class CalcPairsCore
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::size_t world_rank;  
  const std::size_t world_size;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

  FILE *output;
  
  void open_file(const std::string &file_name);
  void close_file();
  void record_pair_count(const std::vector<std::size_t> &count, FILE *stream) const;

public:
  CalcPairsCore(const DataContainer &_data,
                const std::vector<std::size_t> &_free_rows,
                const std::vector<std::size_t> &_free_cols);
  ~CalcPairsCore();

  void work();
};



#endif