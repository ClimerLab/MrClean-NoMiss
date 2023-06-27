#include "AddRowGreedy.h"
#include <assert.h>
#include <algorithm>

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
AddRowGreedy::AddRowGreedy(const DataContainer &_data) :  data(&_data),
                                                          num_rows(data->get_num_data_rows()),
                                                          num_cols(data->get_num_data_cols()),
                                                          best_obj_value(0),
                                                          best_num_rows(0),
                                                          columns(num_cols, true),
                                                          num_included_cols(num_cols),
                                                          alphas(num_rows),
                                                          excluded_rows(num_rows) {
  // Initialize alphas & set all rows to excluded
  for (std::size_t i = 0; i < num_rows; ++i) {
    alphas[i] = data->get_num_valid_in_row(i);
    excluded_rows[i] = i;
  }  
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
AddRowGreedy::~AddRowGreedy() {}


//------------------------------------------------------------------------------
// Finds the greedy solution to the cleaning problem. To begin, all rows are 
// excluded from the solution. Rows are iteratvely added to the solution. Upon
// each row inclusion, columns with missing data in the included row are
// removed. The number of valid elements in the solution is calculated after
// each row inclusion and the best is later returned as the greedy objective
// value.
//------------------------------------------------------------------------------
void AddRowGreedy::solve() {
  // Loop until all rows are included
  while (!excluded_rows.empty()) {
    // Find next row to include
    std::size_t nextRow = get_next_row();

    // Move row from exluded to included
    include_row(nextRow);

    // Update alphas
    update_alphas(nextRow);

    // Calculate new objective
    std::size_t cur_obj = calc_obj();

    // If current objective value is better than incumbent, update obj_value and num_rows
    if (cur_obj > best_obj_value) {
      best_obj_value = cur_obj;
      best_num_rows = included_rows.size();
    }
  }
}

//------------------------------------------------------------------------------
// Calculates the number of valid elements in the solution by mulitplying the
// number of included rows by the number of included columns.
//------------------------------------------------------------------------------
std::size_t AddRowGreedy::calc_obj() const {
  return included_rows.size() * num_included_cols;
}

//------------------------------------------------------------------------------
// Selects the next row to add to the solution. First, the row with the best
// alpha is identified. If only 1 row contains the best alpha, it is selected
// as the next row to add. If multiple rows have the best alpha, determine which
// row would removed the most missing elements in rows with similar alpha. Pick
// the row that will remove the most missing data. Any secondary ties are broken
// based on the numbe of valid elements in the rows with missing data.
//------------------------------------------------------------------------------
std::size_t AddRowGreedy::get_next_row() {
  std::size_t next_row = excluded_rows[0];
  std::size_t best_alpha = alphas[next_row];

  // Find the best alpha and the row that contains it. Note if multiple rows
  // contain best alpha
  bool multi_found = false;
  for (std::size_t k = 0; k < excluded_rows.size(); ++k) {
    std::size_t i = excluded_rows[k];
    if (alphas[i] > best_alpha) {
      best_alpha = alphas[i];
      next_row = i;
    } else if (alphas[i] == best_alpha) {
      multi_found = true;
    }
  }

  // Check if multiple rows contain best alpha
  if (multi_found) {
    // Get all rows with best alpha
    std::vector<std::size_t> possible_rows;
    for (auto i : excluded_rows) {
      if (alphas[i] == best_alpha) {
        possible_rows.push_back(i);
      }
    }

    // Allocate memory for stats for each possible row
    std::vector<std::size_t> worst_num_miss(possible_rows.size(), 0);
    std::vector<std::size_t> alpha_sum(possible_rows.size(), 0);
    std::size_t overall_worst_num_miss = 0;
    
    // Loop through all rows with best alpha
    for (std::size_t i_pos_row = 0; i_pos_row < possible_rows.size(); ++i_pos_row) {
      std::size_t i = possible_rows[i_pos_row];
    
      // Loop though all columns
      for (std::size_t j = 0; j < num_cols; ++j) {
        // Check that column is included and the element (i,j) is missing
        // NOTE: These columns would be exluded if the row was added to the solution
        if (columns[j] && data->is_data_na(i,j)) {      

          // Count the number of missing elements in currently excluded rows & included columns, that would be
          // removed if row 'i' is included. Also sum the alphas of the rows with these missing elements. Only
          // consider rows with alphas within 3 of the row 'i'
          // Loop through all excluded rows
          std::size_t num_miss = 0;
          for (auto ii : excluded_rows) {
            // Check that the data is missing in the 'ii' row and that alpha value of row ii is close to bestAlpha            
            if (data->is_data_na(ii, j) && (best_alpha - alphas[ii] < 3)) {
              ++num_miss;
              alpha_sum[i_pos_row] += alphas[ii];
            }
          }

          if (num_miss > worst_num_miss[i_pos_row]) {
            worst_num_miss[i_pos_row] = num_miss;
          }

          if (num_miss > overall_worst_num_miss) {
            overall_worst_num_miss = num_miss;
            next_row = i;
          }
        }
      }
    }
    
    // Select the row, from the possible_rows, that would remove the most missing elements.
    // Break any ties with the row that has the best alpha_sum
    std::size_t best_alpha_sum = 0;
    for (std::size_t i = 0; i < worst_num_miss.size(); ++i) {
      if ((worst_num_miss[i] == overall_worst_num_miss) && (alpha_sum[i] > best_alpha_sum)) {
        next_row = possible_rows[i];
        best_alpha_sum = alpha_sum[i];        
      }
    }
  }

  return next_row;
}

//------------------------------------------------------------------------------
// This function adds the 'row' to the included row vector and removes it from
// the excluded row vector. If 'row' is not found in the excluded vector an 
// error is reported.
//------------------------------------------------------------------------------
void AddRowGreedy::include_row(const std::size_t row) {
  // Add row to included set
  included_rows.push_back(row);

  // Find index of row in excluded set
  for (std::size_t i = 0; i < excluded_rows.size(); ++i) {
    if (excluded_rows[i] == row) {
      excluded_rows.erase(excluded_rows.begin() + i);
      return;
    }
  }

  fprintf(stderr, "ERROR - Could not find row %lu in excluded set\n", row);
}


//------------------------------------------------------------------------------
// This function receives the latest row that was added / included in the
// solution. All currently included columns are checked for missing data
// corresponding to the received row. If a column has missing data in the
// received row, it is removed from the solution. The alpha value (number of
// valid elements in included columns) for each excluded row is updated based on
// any removed columns.
//------------------------------------------------------------------------------
void AddRowGreedy::update_alphas(const std::size_t row) {
  // Loop through all columns
  for (std::size_t j = 0; j < columns.size(); ++j) {
    // Check if column is currently in solution & is missing in 'row'
    if (columns[j] && data->is_data_na(row, j)) {
      columns[j] = false;
      --num_included_cols;

      // Loop through all excluded rows
      for (auto i : excluded_rows) {
        // Check if element in NOT missing
        if (!data->is_data_na(i,j)) {
          --alphas[i]; // Decrement alpha
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
// Returns a boolean vector where elements are 'true' if the corresponding row
// is kept and 'false' if the row is removed.
//------------------------------------------------------------------------------
std::vector<bool> AddRowGreedy::get_rows_to_keep() const {
  std::vector<bool> rows_to_keep(num_rows, false);

  for (std::size_t i = 0; i < best_num_rows; ++i) {
    std::size_t row = included_rows[i];
    rows_to_keep[row] = true;
  }

  return rows_to_keep;
}

//------------------------------------------------------------------------------
// Returns a boolean vector where elements are 'true' if the corresponding
// column is kept and 'false' if the column is removed.
//------------------------------------------------------------------------------
std::vector<bool> AddRowGreedy::get_cols_to_keep() const {
  std::vector<bool> cols_to_keep(num_cols, true);

  for (std::size_t j = 0; j < num_cols; ++j) {
    for (std::size_t k = 0; k < best_num_rows; ++k) {
      std::size_t i = included_rows[k];
      if (data->is_data_na(i,j)) {
        cols_to_keep[j] = false;
        break;
      }
    }
  }

  return cols_to_keep;
}

//------------------------------------------------------------------------------
// Returns the number of rows kept in the best solution.
//------------------------------------------------------------------------------
std::size_t AddRowGreedy::get_num_rows_to_keep() const {
  return best_num_rows;
}

//------------------------------------------------------------------------------
// Returns the number of columns kept in the best solution.
//------------------------------------------------------------------------------
std::size_t AddRowGreedy::get_num_cols_to_keep() const {
  std::size_t num_cols_to_keep = num_cols;

  for (std::size_t j = 0; j < num_cols; ++j) {
    for (std::size_t k = 0; k < best_num_rows; ++k) {
      std::size_t i = included_rows[k];
      if (data->is_data_na(i,j)) {
        --num_cols_to_keep;
        break;
      }
    }
  }

  return num_cols_to_keep;
}