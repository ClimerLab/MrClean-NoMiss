#include "ElementSolverController.h"
#include <assert.h>
#include <algorithm>
#include "Parallel.h"
#include "Utils.h"
#include "NoMissSummary.h"

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
ElementSolverController::ElementSolverController(const DataContainer &_data) :  data(&_data),
                                                                                num_rows(data->get_num_data_rows()),
                                                                                num_cols(data->get_num_data_cols()),
                                                                                world_size(Parallel::get_world_size()),
                                                                                best_num_elements(0) {
  for (std::size_t i = world_size - 1; i > 0; --i) {
    available_workers.push(i);
  }

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

  noMissSummary::write_solution_to_file("Element.sol", best_rows_to_keep, best_cols_to_keep);
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
ElementSolverController::~ElementSolverController() {}


void ElementSolverController::work() {
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

  // Check every row_sum value
  for (auto rsp : sorted_row_sums) {
    std::size_t row_sum = rsp.first;

    // Calculate minimum number of columns in solution to improve score
    std::size_t min_cols = (best_num_elements / row_sum) + 1;

    // Check if 'min_cols' is greater than 'max_cols_possilbe'
    if (min_cols > max_cols_possible[row_sum-1]) {
      continue;
    }

    valid_col.clear();
    valid_col.resize(free_cols.size(), 1);

    const std::size_t min_free_cols = min_cols - forced_one_cols.size();
    std::size_t free_col_sum_bound = free_cols.size();
    bool set_new_col_to_zero = true;

    while(set_new_col_to_zero) {
      set_new_col_to_zero = false;

      // Loop through all free columns
      for (std::size_t j = 0; j < free_cols.size(); ++j) {
        if (!valid_col[j]) continue; // If current column is alread on valid, continue to next column

        // Check if the column contains enough valid elements compare to the rowSum
        if (data->get_num_valid_in_col(free_cols[j]) < row_sum) {
          valid_col[j] = 0;
          --free_col_sum_bound;
          continue;
        }

        // Get the number of pairs, from valid columns, that are >= the threshold
        std::size_t count = col_pairs.getNumPairsGteThresh(j, row_sum, valid_col);

        // Check if the number of valid columns is below the cutoff
        // We subtract 1 to account for the current column contributing to the cutoff
        if (count < min_free_cols - 1) {
          set_new_col_to_zero = true;
          valid_col[j] = 0;
          --free_col_sum_bound;
        }
      }
      
      // If the number of remaining free columns is below the cuttoff, break from the loop
      if (free_col_sum_bound < min_free_cols - 1) {
        break;
      }
    }

    valid_row.clear();
    valid_row.resize(free_rows.size(), 1);

    const std::size_t min_free_rows = row_sum - forced_one_rows.size();
    bool set_new_row_to_zero = true;
    std::size_t free_row_sum_bound = free_rows.size();
    while(set_new_row_to_zero) {
      set_new_row_to_zero = false;
      
      for (std::size_t i = 0; i < free_rows.size(); ++i) {
        if (!valid_row[i]) continue;

        // Check if the row contains enough valid elements compare to the minCols
        if (data->get_num_valid_in_row(free_rows[i]) < min_cols) {
          valid_row[i] = 0;
          --free_row_sum_bound;
          continue;
        }

        std::size_t count = row_pairs.getNumPairsGteThresh(i, min_cols, valid_row);
        if (count < min_free_rows - 1) {
          set_new_row_to_zero = true;
          valid_row[i] = 0;
          --free_row_sum_bound;
        }
      }

      if (free_row_sum_bound < min_free_rows - 1) {
        break;
      }
    }

    if (free_row_sum_bound < min_free_rows) {
      continue;
    }

    send_problem(row_sum, min_cols);
  }
}

void ElementSolverController::signal_workers_to_end() {
  char signal = 0;
  for (std::size_t i = 1; i < world_size; ++i) {
    MPI_Send(&signal, 1, MPI_CHAR, i, Parallel::CONVERGE_TAG, MPI_COMM_WORLD);
  }
}

void ElementSolverController::wait_for_workers() {
  receive_completion();
}

bool ElementSolverController::workers_still_working() {
  return (available_workers.size() < (world_size - 1));
}

std::vector<int> ElementSolverController::get_rows_to_keep() const {
  return best_rows_to_keep;
}

std::vector<int> ElementSolverController::get_cols_to_keep() const {
  return best_cols_to_keep;
}

std::size_t ElementSolverController::get_num_rows_to_keep() const {
  std::size_t count = 0;
  for (auto r : best_rows_to_keep) {
    count += r;
  }
  return count;
}

std::size_t ElementSolverController::get_num_cols_to_keep() const {
  std::size_t count = 0;
  for (auto c : best_cols_to_keep) {
    count += c;
  }
  return count;
}

FILE* ElementSolverController::open_file_for_read(const std::string &file_name) const {
  FILE* input;
  if ((input = fopen(file_name.c_str(), "r")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file %s\n", file_name.c_str());
    exit(1);
  }
  return input;
}

void ElementSolverController::read_forced_one_rows() {
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

void ElementSolverController::read_forced_one_cols() {
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

void ElementSolverController::read_free_rows() {
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

void ElementSolverController::read_free_cols() {
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

void ElementSolverController::send_problem(const std::size_t row_sum,
                                           const std::size_t min_cols) {
  if (available_workers.empty()) { // wait for a free worker
    receive_completion();
  }
  
  assert(!available_workers.empty()); // Cannot send problem with no available workers

  const int worker = available_workers.top();
  // fprintf(stderr, "Sending rowsum=%lu to worker %d\n", row_sum, worker);

  // Send the row_sum for the problem
  MPI_Send(&row_sum, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send the minimum number of columns for the problem
  MPI_Send(&min_cols, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  MPI_Send(&valid_row[0], valid_row.size(), MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  MPI_Send(&valid_col[0], valid_col.size(), MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Make the worker unavailable
  available_workers.pop();
  unavailable_workers.insert(worker);
}

void ElementSolverController::receive_completion() {
  assert(available_workers.size() < world_size - 1); // Cannot receive problem when no workers are working

  MPI_Status status;

  // Receive the solution
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  // Receive row/col indicator
  std::size_t row_sum;
  MPI_Recv(&row_sum, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // Receive obj_value
  std::size_t obj_value;
  MPI_Recv(&obj_value, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  std::size_t num_elements = row_sum * obj_value;  
  if (num_elements > best_num_elements) {
    MPI_Recv(&best_rows_to_keep[0], num_rows, MPI_INT, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
    MPI_Recv(&best_cols_to_keep[0], num_cols, MPI_INT, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
    best_num_elements = num_elements;

    noMissSummary::write_solution_to_file("Element.sol", best_rows_to_keep, best_cols_to_keep);
  }
  
  // Make the workers available again
  available_workers.push(status.MPI_SOURCE);
  unavailable_workers.erase(status.MPI_SOURCE);
}