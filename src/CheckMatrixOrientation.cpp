#include <string>
#include "BinContainer.h"

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

  BinContainer data(data_file, na_symbol, num_header_rows, num_header_cols);
  fprintf(stderr, "File has %lu data rows and %lu data columns\n", data.get_num_data_rows(), data.get_num_data_cols());

  FILE *results;
  if ((results = fopen("results.txt", "w")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open results file.\n");
    exit(1);
  }

  size_t lastindex = data_file.find_last_of(".");
  std::string out_file;
  if (data.get_num_data_rows() > data.get_num_data_cols()) {
     out_file = data_file.substr(0, lastindex);
    fprintf(stderr, "Transposing matrix\n");
    out_file += "_T.tsv";
    data.write_orig_transpose(out_file);

    fprintf(results, "DATA_FILE=%s\n", out_file.c_str());
    fprintf(results, "NUM_HEADER_ROWS=%lu\n", data.get_num_header_cols());
    fprintf(results, "NUM_HEADER_COLS=%lu\n", data.get_num_header_rows());
    fprintf(results, "NUM_TOTAL_ROWS=%lu\n", data.get_num_header_cols() + data.get_num_data_cols());
    fprintf(results, "NUM_TOTAL_COLS=%lu\n", data.get_num_header_rows() + data.get_num_data_rows());
  } else {
    out_file = data_file;

    fprintf(results, "DATA_FILE=%s\n", out_file.c_str());
    fprintf(results, "NUM_HEADER_ROWS=%lu\n", data.get_num_header_rows());
    fprintf(results, "NUM_HEADER_COLS=%lu\n", data.get_num_header_cols());
    fprintf(results, "NUM_TOTAL_ROWS=%lu\n", data.get_num_header_rows() + data.get_num_data_rows());
    fprintf(results, "NUM_TOTAL_COLS=%lu\n", data.get_num_header_cols() + data.get_num_data_cols());
  }
  fclose(results);


  FILE *stats;
  std::string stats_str = "marix_stats.csv";
  if ((stats = fopen(stats_str.c_str(), "a+")) == nullptr) {
    fprintf(stderr, "Could not open file %s\n", stats_str.c_str());
    exit(1);
  }
  if (data.get_num_data_rows() > data.get_num_data_cols()) {
    fprintf(stats, "%s,%lu,%lu,%lu\n", data_file.c_str(), data.get_num_data_cols(), data.get_num_data_rows(), data.get_num_valid_data());
  } else {
    fprintf(stats, "%s,%lu,%lu,%lu\n", data_file.c_str(), data.get_num_data_rows(), data.get_num_data_cols(), data.get_num_valid_data());
  }
  fclose(stats);
}