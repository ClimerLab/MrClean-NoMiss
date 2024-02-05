#ifndef NO_MISS_SUMMARY_H
#define NO_MISS_SUMMARY_H

#include <string>
#include <vector>
#include "BinContainer.h"

namespace noMissSummary {
  void summarize_results(const BinContainer &data,
                         const std::string &na_symbol,
                         const std::string &alg_name,
                         const double elapsed_cpu_time,
                         const std::size_t num_rows_kept,
                         const std::size_t num_cols_kept,
                         const std::vector<bool> rows_to_keep,
                         const std::vector<bool> cols_to_keep);
  
  void summarize_results(const BinContainer &data,
                         const std::string &na_symbol,
                         const std::string &alg_name,
                         const double elapsed_cpu_time,
                         const std::size_t num_rows_kept,
                         const std::size_t num_cols_kept,
                         const std::vector<int> rows_to_keep,
                         const std::vector<int> cols_to_keep);

  void write_stats_to_file(const std::string &file_name,
                           const std::string &data_file,
                           const double time,
                           const std::size_t num_valid_element,
                           const std::size_t num_rows_kept,
                           const std::size_t num_cols_kept);

  void write_solution_to_file(const std::string &file_name,
                              const std::vector<bool> &rows_to_keep,
                              const std::vector<bool> &cols_to_keep);

  void write_solution_to_file(const std::string &file_name,
                              const std::vector<int> &rows_to_keep,
                              const std::vector<int> &cols_to_keep);

  void read_solution_from_file(const std::string &file_name,
                               std::vector<bool> &rows_to_keep,
                               std::vector<bool> &cols_to_keep);

  void read_solution_from_file(const std::string &file_name,
                               std::vector<int> &rows_to_keep,
                               std::vector<int> &cols_to_keep);
}
#endif