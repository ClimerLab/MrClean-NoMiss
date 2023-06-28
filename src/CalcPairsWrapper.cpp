#include <stdio.h>
#include <string>

#include "DataContainer.h"
#include "Timer.h"
#include "CalcPairs.h"

int main(int argc, char* argv[]) {
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
  timer.start();
  // Read in data
  DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);

  CalcPairs calcPairs(data);
  calcPairs.calc_free_row_pairs();
  calcPairs.calc_free_col_pairs();

  timer.stop();

  std::string file_name = "CalcPairs.csv";
  FILE* output;
  if((output = fopen(file_name.c_str(), "a+")) == nullptr) {
    fprintf(output, "ERROR - Could not open file (%s)\n", file_name.c_str());
    exit(1);
  }
  fprintf(output, "%s,%lf\n", data_file.c_str(), timer.elapsed_cpu_time());
  fclose(output);

  fprintf(stderr, "Summary of CalcPairs\n");
  fprintf(stderr, "\tTook %lf seconds\n\n", timer.elapsed_cpu_time());

  return 0;
}