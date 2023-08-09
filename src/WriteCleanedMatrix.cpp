#include <string>
#include <vector>
#include "DataContainer.h"
#include "NoMissSummary.h"

int main(int argc, char* argv[]) {
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
  
  DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);
  std::vector<int> best_rows_to_keep, best_cols_to_keep;
  std::vector<int> rows_to_keep(data.get_num_data_rows(), 0);
  std::vector<int> cols_to_keep(data.get_num_data_cols(), 0);
  std::size_t num_elements = 0;

  FILE *test;
  std::string greedy_sol = "AddRowGreedy.sol";
  if ((test = fopen(greedy_sol.c_str(), "r")) != nullptr) {
    fclose(test);

    noMissSummary::read_solution_from_file(greedy_sol, rows_to_keep, cols_to_keep);

    std::size_t tmp_num_elements = data.get_num_valid_data_kept(rows_to_keep, cols_to_keep);

    if (tmp_num_elements > num_elements) {
      best_rows_to_keep = rows_to_keep;
      best_cols_to_keep = cols_to_keep;
      num_elements = tmp_num_elements;
    }
  }

  std::string row_col_sol = "RowCol.sol";
  if ((test = fopen(row_col_sol.c_str(), "r")) != nullptr) {
    fclose(test);
    
    noMissSummary::read_solution_from_file(row_col_sol, rows_to_keep, cols_to_keep);

    std::size_t tmp_num_elements = data.get_num_valid_data_kept(rows_to_keep, cols_to_keep);
    if (tmp_num_elements > num_elements) {
      best_rows_to_keep = rows_to_keep;
      best_cols_to_keep = cols_to_keep;
      num_elements = tmp_num_elements;
    }
  }

    std::string element_sol = "Element.sol";
  if ((test = fopen(element_sol.c_str(), "r")) != nullptr) {
    fclose(test);
    
    noMissSummary::read_solution_from_file(element_sol, rows_to_keep, cols_to_keep);

    std::size_t tmp_num_elements = data.get_num_valid_data_kept(rows_to_keep, cols_to_keep);
    if (tmp_num_elements > num_elements) {
      best_rows_to_keep = rows_to_keep;
      best_cols_to_keep = cols_to_keep;
      num_elements = tmp_num_elements;
    }
  }

  if (num_elements == 0) {
    fprintf(stderr, "ERROR - No valid solutions found\n");
    exit(1);
  }  
  
  fprintf(stderr, "Writing cleaned matrix for %s\n", data_file.c_str());
  fprintf(stderr, "Matrix contains %lu valid elements\n", num_elements);
  size_t lastindex = data_file.find_last_of("."); 
  std::string cleaned_file = data_file.substr(0, lastindex); 
  cleaned_file += "_cleaned.tsv";
  data.write(cleaned_file, rows_to_keep, cols_to_keep);

  std::size_t num_rows_kept = 0, num_cols_kept;
  for (auto r : best_rows_to_keep) {
    num_rows_kept += r;
  }
  for (auto c : best_cols_to_keep) {
    num_cols_kept += c;
  }
  noMissSummary::write_stats_to_file("Best.csv", data_file, 0, num_elements, num_rows_kept, num_cols_kept);

  return 0;
}