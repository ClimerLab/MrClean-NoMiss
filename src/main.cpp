#include <cstdio>
#include <cstdlib>
#include <string>
#include "DataContainer.h"
#include "RowColLpSolver.h"
#include "Timer.h"
#include "ConfigParser.h"
#include "AddRowGreedy.h"

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

int main(int argc, char *argv[]) {
  // Check user input
  if (!((argc == 3) || (argc == 5))) {
    fprintf(stderr, "Usage: %s <data_file> <na_symbol>\n", argv[0]);
    exit(1);
  }
  std::string data_file(argv[1]);
  std::string na_symbol(argv[2]);
  std::size_t num_header_rows = 1;
  std::size_t num_header_cols = 1;

  if (argc == 5) {
    num_header_rows = std::stoul(argv[3]);
    num_header_cols = std::stoul(argv[4]);
  }
  
  ConfigParser parser("config.cfg");
  const bool PRINT_SUMMARY = parser.getBool("PRINT_SUMMARY");
  const bool WRITE_STATS = parser.getBool("WRITE_STATS");
  const bool RUN_GREEDY = parser.getBool("RUN_GREEDY");
  const bool RUN_ROW_COL = parser.getBool("RUN_ROW_COL");

  Timer timer;

  // Read in data
  DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);

  // Construct & solve RowCol LP
  std::size_t rc_rows = 0, rc_cols = 0, rc_val_elements = 0;
  double rc_time = 0.0;
  std::vector<bool> rc_rows_to_keep(data.get_num_data_rows(), false), rc_cols_to_keep(data.get_num_data_cols(), false);
  if (RUN_ROW_COL) {
    RowColLpSolver rc_solver(data);
    timer.start();
    rc_solver.solve();
    timer.stop();

    rc_rows_to_keep = rc_solver.get_rows_to_keep();
    rc_cols_to_keep = rc_solver.get_cols_to_keep();
    rc_rows = rc_solver.get_num_rows_to_keep();
    rc_cols = rc_solver.get_num_cols_to_keep();
    rc_time = timer.elapsed_cpu_time();
    rc_val_elements = data.get_num_valid_data_kept(rc_rows_to_keep, rc_cols_to_keep);

    // Record results
    if (PRINT_SUMMARY) {
      summarize_results(data, na_symbol, "RowColLp", rc_time, rc_rows, rc_cols, rc_rows_to_keep, rc_cols_to_keep);
    }    
  }

  // Construct and solve greedy
  std::size_t greedy_rows = 0, greedy_cols = 0, greedy_val_elements = 0;
  double greedy_time = 0.0;
  std::vector<bool> greedy_rows_to_keep(data.get_num_data_rows(), false), greedy_cols_to_keep(data.get_num_data_cols(), false);
  if (RUN_GREEDY) {
    timer.restart();
    AddRowGreedy ar_greedy(data);
    ar_greedy.solve();
    timer.stop();

    greedy_rows_to_keep = ar_greedy.get_rows_to_keep();
    greedy_cols_to_keep = ar_greedy.get_cols_to_keep();
    greedy_rows = ar_greedy.get_num_rows_to_keep();
    greedy_cols = ar_greedy.get_num_cols_to_keep();
    greedy_time = timer.elapsed_cpu_time();
    greedy_val_elements = data.get_num_valid_data_kept(rc_rows_to_keep, rc_cols_to_keep);

    if (PRINT_SUMMARY) {
      summarize_results(data, na_symbol, "AddRowGreedy", greedy_time, greedy_rows, greedy_cols, greedy_rows_to_keep, greedy_cols_to_keep);
    }
  }

  // Wrtie statistics to file
  if (WRITE_STATS) {
    if (RUN_ROW_COL) {
      write_stats_to_file("RowCol_summary.csv", data_file, rc_time, rc_val_elements, rc_rows, rc_cols);
    }
    if (RUN_GREEDY) {
      write_stats_to_file("Greedy_summary.csv", data_file, greedy_time, greedy_val_elements, greedy_rows, greedy_cols);
    }
  }
  

  return 0;
}

//------------------------------------------------------------------------------
// Print summary of algorithm to screen
//------------------------------------------------------------------------------
void summarize_results(const DataContainer &data,
                       const std::string &na_symbol,
                       const std::string &alg_name,
                       const double elapsed_cpu_time,
                       const std::size_t num_rows_kept,
                       const std::size_t num_cols_kept,
                       const std::vector<bool> rows_to_keep,
                       const std::vector<bool> cols_to_keep)
{
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
void write_stats_to_file(const std::string &file_name,
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