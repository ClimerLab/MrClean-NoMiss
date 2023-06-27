#include <cstdio>
#include <cstdlib>
#include <string>
#include "DataContainer.h"
#include "Timer.h"
#include "ConfigParser.h"
#include "NoMissSummary.h"
#include "AddRowGreedy.h"

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

  // Construct & solve AddRowGreedy problem
  std::size_t greedy_rows = 0, greedy_cols = 0, greedy_val_elements = 0;
  double greedy_time = 0.0;
  std::vector<bool> greedy_rows_to_keep(data.get_num_data_rows(), false), greedy_cols_to_keep(data.get_num_data_cols(), false);
  
  timer.restart();
  AddRowGreedy ar_greedy(data);
  ar_greedy.solve();
  timer.stop();

  greedy_rows_to_keep = ar_greedy.get_rows_to_keep();
  greedy_cols_to_keep = ar_greedy.get_cols_to_keep();
  greedy_rows = ar_greedy.get_num_rows_to_keep();
  greedy_cols = ar_greedy.get_num_cols_to_keep();
  greedy_time = timer.elapsed_cpu_time();
  greedy_val_elements = data.get_num_valid_data_kept(greedy_rows_to_keep, greedy_cols_to_keep);

  if (PRINT_SUMMARY) {
    noMissSummary::summarize_results(data, na_symbol, "AddRowGreedy", greedy_time, greedy_rows, greedy_cols, greedy_rows_to_keep, greedy_cols_to_keep);
  }
  

    // Wrtie statistics to file
  if (WRITE_STATS) {
    noMissSummary::write_stats_to_file("Greedy_summary.csv", data_file, greedy_time, greedy_val_elements, greedy_rows, greedy_cols);
  }

  return 0;
}