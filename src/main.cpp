#include <cstdio>
#include <cstdlib>
#include <string>
#include "DataContainer.h"
#include "RowColLpSolver.h"
#include "Timer.h"

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
  Timer timer;

  // Read in data
  DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);
  
  // Construct & LP
  RowColLpSolver rc_solver(data);
  timer.start();
  rc_solver.solve();
  timer.stop();

  // Record results
  fprintf(stderr, "Num rows kept: %lu\n", rc_solver.get_num_rows_to_keep());
  fprintf(stderr, "Num cols kept: %lu\n", rc_solver.get_num_cols_to_keep());
  fprintf(stderr, "Num elements kept: %lu\n", rc_solver.get_num_elements_to_keep());  
  fprintf(stderr, "RC time: %lf\n", timer.elapsed_cpu_time());

  return 0;
}