#include <cstdio>
#include <cstdlib>
#include <string>
#include "DataContainer.h"

int main(int argc, char *argv[]) {
  // Check user input
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <data_file> <na_symbol>\n", argv[0]);
    exit(1);
  }
  std::string data_file(argv[1]);
  std::string na_symbol(argv[2]);
  
  // Read in data
  DataContainer data(data_file, na_symbol);

  // Construct LP

  // Solve LP

  return 0;
}