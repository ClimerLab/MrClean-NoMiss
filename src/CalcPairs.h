#ifndef CALC_PAIRS_H
#define CALC_PAIRS_H

#include <vector>
#include <string>

#include "DataContainer.h"

class CalcPairs
{
private:
  const DataContainer *data;
  const std::size_t num_rows;
  const std::size_t num_cols;

  std::vector<std::size_t> forced_one_rows;
  std::vector<std::size_t> forced_one_cols;

  std::vector<std::size_t> free_rows;
  std::vector<std::size_t> free_cols;

  void calc_free_rows();
  void calc_free_cols();

  FILE* openFile(const std::string &file_name) const;
  void record_free_rows() const;
  void record_free_cols() const;

  void record_pair_count(FILE* stream, const std::vector<std::size_t> &pairs) const;

public:
  CalcPairs(const DataContainer &_data);
  ~CalcPairs();

  void calc_free_row_pairs();
  void calc_free_col_pairs();
};

#endif