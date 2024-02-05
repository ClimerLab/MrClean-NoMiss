#include "ElementSolver.h"
#include "AddRowGreedy.h"
#include "ElementIpSolver.h"
#include <algorithm>
#include "Utils.h"

ElementSolver::ElementSolver(const BinContainer &_data,
                             const double _TOL) : data(&_data),
                                                  num_rows(data->get_num_data_rows()),
                                                  num_cols(data->get_num_data_cols()),
                                                  TOL(_TOL),
                                                  best_num_elements(0)
{
  read_forced_one_rows();
  read_forced_one_cols();
  read_free_rows();
  read_free_cols();

  row_pairs.set_size(free_rows.size()-1);
  row_pairs.read("rowPairs.csv");
  
  col_pairs.set_size(free_cols.size()-1);
  col_pairs.read("colPairs.csv");

  AddRowGreedy ar_greedy(*data);
  ar_greedy.solve();

  best_rows_to_keep = ar_greedy.get_rows_to_keep();
  best_cols_to_keep = ar_greedy.get_cols_to_keep();
  best_num_elements = ar_greedy.get_num_rows_to_keep() * ar_greedy.get_num_cols_to_keep();

}

ElementSolver::~ElementSolver() {}

FILE* ElementSolver::open_file_for_read(const std::string &file_name) const {
  FILE* input;
  if ((input = fopen(file_name.c_str(), "r")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file %s\n", file_name.c_str());
    exit(1);
  }
  return input;
}

void ElementSolver::read_forced_one_rows() {
  FILE* input = open_file_for_read("forcedOneRows.txt");
	char tmp_str[50];

  while (true) {
    fscanf(input, "%s", tmp_str);

    if (feof(input)) {
      break;
    }

    forced_one_rows.push_back(std::stoul(tmp_str));
  }

  fclose(input);
}

void ElementSolver::read_forced_one_cols() {
  FILE* input = open_file_for_read("forcedOneCols.txt");
	char tmp_str[50];

  while (true) {
    fscanf(input, "%s", tmp_str);

    if (feof(input)) {
      break;
    }

    forced_one_cols.push_back(std::stoul(tmp_str));
  }

  fclose(input);
}

void ElementSolver::read_free_rows() {
  FILE* input = open_file_for_read("freeRows.txt");
	char tmp_str[50];

  while (true) {
    fscanf(input, "%s", tmp_str);

    if (feof(input)) {
      break;
    }

    free_rows.push_back(std::stoul(tmp_str));
  }

  fclose(input);
}

void ElementSolver::read_free_cols() {
  FILE* input = open_file_for_read("freeCols.txt");
	char tmp_str[50];

  while (true) {
    fscanf(input, "%s", tmp_str);

    if (feof(input)) {
      break;
    }

    free_cols.push_back(std::stoul(tmp_str));
  }

  fclose(input);
}

void ElementSolver::solve() {
  // Order rows by missing number of elements
  std::vector<std::size_t> num_missing(num_rows);
  for (std::size_t i = 0; i < num_rows; ++i) {
    num_missing[i] = data->get_num_invalid_in_row(i);
  }
  std::sort(num_missing.begin(), num_missing.end());

  // Calculate maximum number of kept columns associated with keeping i rows 
  // by subtracting the number of invalid elements of the i-th ordered row from
  // the number of columns
  std::vector<std::size_t> max_cols_possible(num_rows);
  for (std::size_t i = 0; i < num_rows; ++i) {
    max_cols_possible[i] = num_cols - num_missing[i];
  }

  std::size_t best_num_rows = get_num_rows_to_keep();

  std::vector<std::pair<std::size_t, std::size_t>> sorted_row_sums;
  for (std::size_t i = 0; i < num_rows; ++i) {
    if (i+1 > best_num_rows) {
      sorted_row_sums.push_back(std::make_pair(i+1, i + 1 - best_num_rows));
    } else {
      sorted_row_sums.push_back(std::make_pair(i+1, best_num_rows - (i+1)));
    }    
  }
  std::sort(sorted_row_sums.begin(), sorted_row_sums.end(), utils::SortPairBySecondItemIncreasing());

  for (auto rsp : sorted_row_sums) {
    std::size_t row_sum = rsp.first;
  // Check every row_sum value
  // for (std::size_t row_sum = 271; row_sum <= 271; ++row_sum) {
    // Calculate minimum number of columns in solution to improve score
    std::size_t min_cols = (best_num_elements / row_sum) + 1;
    // fprintf(stderr, "min_cols=%lu for row_sum=%lu\n", min_cols, row_sum);

    // Check if 'min_cols' is greater than 'max_cols_possilbe'
    if (min_cols > max_cols_possible[row_sum-1]) {      
      // fprintf(stderr, "Skipping row_sum=%lu (%lu vs. %lu)\n", row_sum, max_cols_possible[row_sum-1], min_cols);
      continue;
    }

    const std::size_t min_free_cols = min_cols - forced_one_cols.size();
    std::size_t free_col_sum_bound = free_cols.size();

    std::vector<bool> valid_col(free_cols.size(), true);
    bool set_new_col_to_zero = true;
    
    while(set_new_col_to_zero) {
      set_new_col_to_zero = false;

      // Loop through all free columns
      for (std::size_t j = 0; j < free_cols.size(); ++j) {        
        if (!valid_col[j]) continue; // If current column is alread on valid, continue to next column

        // Check if the column contains enough valid elements compare to the rowSum
        if (data->get_num_valid_in_col(free_cols[j]) < row_sum) {
          valid_col[j] = false;
          --free_col_sum_bound;
          continue;
        }

        // Get the number of pairs, from valid columns, that are >= the threshold
        std::size_t count = col_pairs.getNumPairsGteThresh(j, row_sum, valid_col);

        // Check if the number of valid columns is below the cutoff
        // We subtract 1 to account for the current column contributing to the cutoff
        if (count < min_free_cols - 1) {
          set_new_col_to_zero = true;
          valid_col[j] = false;
          --free_col_sum_bound;
        }
      }
      
      // If the number of remaining free columns is below the cuttoff, break from the loop
      if (free_col_sum_bound < min_free_cols - 1) {
        // fprintf(stderr, "Skipping row_sum=%lu. Remain free cols below threshold\n", row_sum);
        break;
      }
    }
    // fprintf(stderr, "\tFree col bound: %lu\n", free_col_sum_bound);


    const std::size_t min_free_rows = row_sum - forced_one_rows.size();
    std::vector<bool> valid_row(free_rows.size(), true);
    bool set_new_row_to_zero = true;
    std::size_t free_row_sum_bound = free_rows.size();
    while(set_new_row_to_zero) {
      set_new_row_to_zero = false;
      
      for (std::size_t i = 0; i < free_rows.size(); ++i) {
        if (!valid_row[i]) continue;

        // Check if the row contains enough valid elements compare to the minCols
        if (data->get_num_valid_in_row(free_rows[i]) < min_cols) {
          valid_row[i] = false;
          --free_row_sum_bound;
          continue;
        }

        std::size_t count = row_pairs.getNumPairsGteThresh(i, min_cols, valid_row);
        if (count < min_free_rows - 1) {
          set_new_row_to_zero = true;
          valid_row[i] = false;
          --free_row_sum_bound;
        }
      }

      if (free_row_sum_bound < min_free_rows - 1) {
        break;
      }
    }

    // fprintf(stderr, "\tFree Row Sum Bound: %lu\n", free_row_sum_bound);

    if (free_row_sum_bound < min_free_rows) {
      // fprintf(stderr, "Skipping rowSum=%lu due to num valid rows\n", row_sum);
      continue;
    }


    ElementIpSolver ip_solver(*data,
                              row_sum,
                              min_cols,
                              forced_one_rows,
                              forced_one_cols,
                              free_rows,
                              free_cols);

    for (std::size_t i = 0; i < free_rows.size(); ++i) {
      if (valid_row[i]) {
        auto pairs = row_pairs.getPairsLtThresh(i, min_cols, valid_row);
        if (!pairs.empty()) {
          ip_solver.add_row_pairs_cut(i, pairs);
        }
      } else {
        ip_solver.set_row_to_zero(i);
      }
    }
    for (std::size_t j = 0; j < free_cols.size(); ++j) {
      if (valid_col[j]) {
        auto pairs = col_pairs.getPairsLtThresh(j, row_sum, valid_col);
        if (!pairs.empty()) {
          ip_solver.add_col_pairs_cut(j, pairs);
        }
      } else {
        ip_solver.set_col_to_zero(j);
      }
    }

    ip_solver.solve();

    if (ip_solver.get_num_elements() > best_num_elements) {
      best_num_elements = ip_solver.get_num_elements();
      best_rows_to_keep = ip_solver.get_rows_to_keep();
      best_cols_to_keep = ip_solver.get_cols_to_keep();

      fprintf(stderr, " *** best num elemente: %lu ***\n", best_num_elements);
      fprintf(stderr, "  Num rows: %lu\n", row_sum);
      fprintf(stderr, "  Num cols: %lu\n", ip_solver.get_obj_value());
    }
  } 
}

std::vector<bool> ElementSolver::get_rows_to_keep() const {
  return best_rows_to_keep;
}

std::vector<bool> ElementSolver::get_cols_to_keep() const {
  return best_cols_to_keep;
}

std::size_t ElementSolver::get_num_rows_to_keep() const {
  std::size_t count = 0;
  for (auto r : best_rows_to_keep) {
    count += r;
  }
  return count;
}

std::size_t ElementSolver::get_num_cols_to_keep() const {
  std::size_t count = 0;
  for (auto c : best_cols_to_keep) {
    count += c;
  }
  return count;
}