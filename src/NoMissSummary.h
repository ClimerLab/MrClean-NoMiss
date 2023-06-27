#ifndef NO_MISS_SUMMARY_H
#define NO_MISS_SUMMARY_H

#include <string>
#include <vector>
#include "DataContainer.h"

namespace noMissSummary {
  void summarize_results(const DataContainer &data,
                       const std::string &na_symbol,
                       const std::string &alg_name,
                       const double elapsed_cpu_time,
                       const std::size_t num_rows_kept,
                       const std::size_t num_cols_kept,
                       const std::vector<bool> rows_to_keep,
                       const std::vector<bool> cols_to_keep);

void write_stats_to_file(const std::string &file_name,
                         const std::string &data_file,
                         const double time,
                         const std::size_t num_valid_element,
                         const std::size_t num_rows_kept,
                         const std::size_t num_cols_kept);
}

#endif