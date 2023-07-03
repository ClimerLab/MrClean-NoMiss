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
void noMissSummary::write_solution_to_file(const std::string &file_name,
                                           const std::vector<bool> &rows_to_keep,
                                           const std::vector<bool> &cols_to_keep) {
  assert(rows_to_keep.size() > 0);
  assert(cols_to_keep.size() > 0);

  FILE *output;

  if((output = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }

  for (auto r : rows_to_keep) {
    fprintf(output, r ? "1\n" : "0\n");
  }
  for (auto c : cols_to_keep) {
    fprintf(output, c ? "1\n" : "0\n");
  }

  fclose(output);
}

//------------------------------------------------------------------------------
// Writes the rows_to_keep and cols_to_keep to a file.
//------------------------------------------------------------------------------
void noMissSummary::write_solution_to_file(const std::string &file_name,
                                           const std::vector<int> &rows_to_keep,
                                           const std::vector<int> &cols_to_keep) {
  assert(rows_to_keep.size() > 0);
  assert(cols_to_keep.size() > 0);

  FILE *output;

  if((output = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }

  for (auto r : rows_to_keep) {
    fprintf(output, "%d\n", r);
  }
  for (auto c : cols_to_keep) {
    fprintf(output, "%d\n", c);
  }

  fclose(output);
}

//------------------------------------------------------------------------------
// Reads the rows_to_keep and cols_to_keep from a file.
//------------------------------------------------------------------------------
void noMissSummary::read_solution_from_file(const std::string &file_name,
                                            std::vector<bool> &rows_to_keep,
                                            std::vector<bool> &cols_to_keep) {
  FILE *input;

  if((input = fopen(file_name.c_str(), "r")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }
  char tmp_str[50];

  std::size_t idx = 0;
  while (true) {
    fscanf(input, "%s", tmp_str);

    if (feof(input)) {
      break;
    }

    if (idx < rows_to_keep.size()) {
      rows_to_keep[idx] = static_cast<bool>(std::stoi(tmp_str));
    } else {
      cols_to_keep[idx - rows_to_keep.size()] = static_cast<bool>(std::stoi(tmp_str));
    }
    ++idx;
  }

  fclose(input);

  if (idx != rows_to_keep.size() + cols_to_keep.size()) {
    fprintf(stderr, "ERROR - Input file doesn't contain the expected number of elements\n");
    exit(1);
  }
}

//------------------------------------------------------------------------------
// Reads the rows_to_keep and cols_to_keep from a file.
//------------------------------------------------------------------------------
void noMissSummary::read_solution_from_file(const std::string &file_name,
                                            std::vector<int> &rows_to_keep,
                                            std::vector<int> &cols_to_keep) {
  FILE *input;
  
  if((input = fopen(file_name.c_str(), "r")) == nullptr) {
    fprintf(stderr, "Could not open file (%s)", file_name.c_str());
    exit(EXIT_FAILURE);
  }
  char tmp_str[50];

  std::size_t idx = 0;
  while (true) {
    fscanf(input, "%s", tmp_str);

    if (feof(input)) {
      break;
    }

    if (idx < rows_to_keep.size()) {
      rows_to_keep[idx] = std::stoi(tmp_str);
    } else {
      cols_to_keep[idx - rows_to_keep.size()] = std::stoi(tmp_str);
    }
    ++idx;
  }

  fclose(input);

  if (idx != rows_to_keep.size() + cols_to_keep.size()) {
    fprintf(stderr, "ERROR - Input file doesn't contain the expected number of elements\n");
    exit(1);
  }
}