#include "NoMissSummary.h"
#include <assert.h>

//------------------------------------------------------------------------------
// Print summary of algorithm to screen
//------------------------------------------------------------------------------
void noMissSummary::summarize_results(const DataContainer &data,
                                      const std::string &na_symbol,
                                      const std::string &alg_name,
                                      const double elapsed_cpu_time,
                                      const std::size_t num_rows_kept,
                                      const std::size_t num_cols_kept,
                                      const std::vector<bool> rows_to_keep,
                                      const std::vector<bool> cols_to_keep) {
  fprintf(stderr, "Summary of %s\n", alg_name.c_str());
  fprintf(stderr, "\tTook %lf seconds\n", elapsed_cpu_time);
  fprintf(stderr, "\tNum rows after cleaning: %lu\n", num_rows_kept);
  fprintf(stderr, "\tNum cols after cleaning: %lu\n", num_cols_kept);
  fprintf(stderr, "\tNumber of valid elements after cleaning: %lu\n", data.get_num_valid_data_kept(rows_to_keep, cols_to_keep));
  fprintf(stderr, "\n");
}

//------------------------------------------------------------------------------
// Print summary of algorithm to screen
//------------------------------------------------------------------------------
void noMissSummary::summarize_results(const DataContainer &data,
                                      const std::string &na_symbol,
                                      const std::string &alg_name,
                                      const double elapsed_cpu_time,
                                      const std::size_t num_rows_kept,
                                      const std::size_t num_cols_kept,
                                      const std::vector<int> rows_to_keep,
                                      const std::vector<int> cols_to_keep) {
  fprintf(stderr, "Summary of %s\n", alg_name.c_str());
  fprintf(stderr, "\tTook %lf seconds\n", elapsed_cpu_time);
  fprintf(stderr, "\tNum rows after cleaning: %lu\n", num_rows_kept);
  fprintf(stderr, "\tNum cols after cleaning: %lu\n", num_cols_kept);
  fprintf(stderr, "\tNumber of valid elements after cleaning: %lu\n", data.get_num_valid_data_kept(rows_to_keep, cols_to_keep));
  fprintf(stderr, "\n");
}

//------------------------------------------------------------------------------
// Write the statistics to a file
//------------------------------------------------------------------------------
void noMissSummary::write_stats_to_file(const std::string &file_name,
                                        const std::string &data_file,
                                        const double time,
                                        const std::size_t num_valid_element,
                                        const std::size_t num_rows_kept,
                                        const std::size_t num_cols_kept) {
  FILE *summary;
  
  if((summary = fopen(file_name.c_str(), "a+")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }

  fprintf(summary, "%s,%lf,%lf,%lu,%lu,%lu\n", data_file.c_str(), time, 0.0, num_valid_element, num_rows_kept, num_cols_kept);

  fclose(summary);
}

//------------------------------------------------------------------------------
// Writes the rows_to_keep and cols_to_keep to a file.
//------------------------------------------------------------------------------
void write_solution_to_file(const std::string &file_name,
                            const std::vector<bool> &rows_to_keep,
                            const std::vector<bool> &cols_to_keep) {
  assert(rows_to_keep.size() > 0);
  assert(cols_to_keep.size() > 0);

  FILE *ouput;
  
  if((ouput = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }
  
  fprintf(ouput, rows_to_keep.at(0) ? "1" : "0");
  for (std::size_t i = 1; i < rows_to_keep.size(); ++i) {
    fprintf(ouput, rows_to_keep.at(i) ? "\t1" : "\t0");
  }
  fprintf(ouput, "\n");

  fprintf(ouput, cols_to_keep.at(0) ? "1" : "0");
  for (std::size_t j = 1; j < cols_to_keep.size(); ++j) {
    fprintf(ouput, cols_to_keep.at(j) ? "\t1" : "\t0");
  }
  fprintf(ouput, "\n");

  fclose(ouput);
}

//------------------------------------------------------------------------------
// Writes the rows_to_keep and cols_to_keep to a file.
//------------------------------------------------------------------------------
void write_solution_to_file(const std::string &file_name,
                            const std::vector<int> &rows_to_keep,
                            const std::vector<int> &cols_to_keep) {
  assert(rows_to_keep.size() > 0);
  assert(cols_to_keep.size() > 0);

  FILE *ouput;
  
  if((ouput = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }
  
  fprintf(ouput, "%d", rows_to_keep.at(0));
  for (std::size_t i = 1; i < rows_to_keep.size(); ++i) {
    fprintf(ouput, "\t%d", rows_to_keep.at(i));
  }
  fprintf(ouput, "\n");

  fprintf(ouput, "%d", cols_to_keep.at(0));
  for (std::size_t j = 1; j < cols_to_keep.size(); ++j) {
    fprintf(ouput, "\t%d", cols_to_keep.at(j));
  }
  fprintf(ouput, "\n");

  fclose(ouput);
}