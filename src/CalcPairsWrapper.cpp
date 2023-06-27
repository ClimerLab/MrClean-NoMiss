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

  // Read in data
  DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);

  CalcPairs calcPairs(data);
  calcPairs.calc_free_row_pairs();
  calcPairs.calc_free_col_pairs();

  return 0;
}