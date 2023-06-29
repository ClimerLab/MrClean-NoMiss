#ifndef CALC_PAIRS_CORE_H
#define CALC_PAIRS_CORE_H

#include <vector>
#include "DataContainer.h"

class CalcPairsCore
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;
  const std::size_t world_rank;  
  const std::size_t world_size;

  std::vector<std::size_t> forced_one_rows;
  std::vector<std::size_t> forced_one_cols;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

public:
  CalcPairsCore(const DataContainer &_data,
                const std::vector<std::size_t> &_free_rows,
                const std::vector<std::size_t> &_free_cols);
  ~CalcPairsCore();

  void work();
};



#endif