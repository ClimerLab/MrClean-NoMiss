#include <cstdio>
#include <cstdlib>
#include <string>
#include "DataContainer.h"
#include "RowColLpSolver.h"
#include "Timer.h"
#include "ConfigParser.h"
#include "NoMissSummary.h"

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

  Timer timer;

  // Read in data
  DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);

    // Construct & solve RowCol LP
  std::size_t rc_rows = 0, rc_cols = 0, rc_val_elements = 0;
  double rc_time = 0.0;
  std::vector<bool> rc_rows_to_keep(data.get_num_data_rows(), false), rc_cols_to_keep(data.get_num_data_cols(), false);

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
    noMissSummary::summarize_results(data, na_symbol, "RowColLp", rc_time, rc_rows, rc_cols, rc_rows_to_keep, rc_cols_to_keep);
  }

    // Wrtie statistics to file
  if (WRITE_STATS) {
    noMissSummary::write_stats_to_file("RowCol_summary.csv", data_file, rc_time, rc_val_elements, rc_rows, rc_cols);
  }

  return 0;
}