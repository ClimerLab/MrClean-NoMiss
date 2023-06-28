#include "CalcPairs.h"

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
CalcPairs::CalcPairs(const DataContainer &_data) :  data(&_data),
                                                    num_rows(data->get_num_data_rows()),
                                                    num_cols(data->get_num_data_cols())
{
  calc_free_rows();
  calc_free_cols();
  record_free_rows();
  record_free_cols();

  // fprintf(stderr, "# forced 1 rows: %lu\n", forced_one_rows.size());
  // fprintf(stderr, "# forced 1 cols: %lu\n", forced_one_cols.size());
  // fprintf(stderr, "# free rows: %lu\n", free_rows.size());
  // fprintf(stderr, "# free cols: %lu\n", free_cols.size());
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
CalcPairs::~CalcPairs() {}

//------------------------------------------------------------------------------
// Add all rows with no invalid elements to the 'forced_one_rows' vector and
// add all other rows to the 'free_rows' vector.
//------------------------------------------------------------------------------
void CalcPairs::calc_free_rows() {
  for (std::size_t i = 0; i < num_rows; ++i) {
    if (data->get_num_invalid_in_row(i) == 0) {
      forced_one_rows.push_back(i);
    } else {
      free_rows.push_back(i);
    }
  }
}

//------------------------------------------------------------------------------
// Add all columns with no invalid elements to the 'forced_one_cols' vector and
// add all other columns to the 'free_cols' vector.
//------------------------------------------------------------------------------
void CalcPairs::calc_free_cols() {
  for (std::size_t j = 0; j < num_cols; ++j) {
    if (data->get_num_invalid_in_col(j) == 0) {
      forced_one_cols.push_back(j);
    } else {
      free_cols.push_back(j);
    }
  }
}


//------------------------------------------------------------------------------
// Tries to open a file based on the passed 'file_name'. Prints an error and 
// exits if file cannot be opened. Returns the stream to file if successfully
// opened.
//------------------------------------------------------------------------------
FILE* CalcPairs::openFile(const std::string &file_name) const {
  FILE* out;
  if ((out = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file %s\n", file_name.c_str());
    exit(1);
  }
  return out;
}

//------------------------------------------------------------------------------
// Records the 'free rows' and 'rows forced to 1' in seperate files.
//------------------------------------------------------------------------------
void CalcPairs::record_free_rows() const {
  FILE* out = openFile("freeRows.txt");
  for (auto r : free_rows) {
    fprintf(out, "%lu\n", r);
  }
  fclose(out);
  
  out = openFile("forcedOneRows.txt");
  for (auto r : forced_one_rows) {
    fprintf(out, "%lu\n", r);
  }
  fclose(out);
}

//------------------------------------------------------------------------------
// Records the 'free cols' and 'cols forced to 1' in seperate files.
//------------------------------------------------------------------------------
void CalcPairs::record_free_cols() const {
  FILE* out = openFile("freeCols.txt");
  for (auto c : free_cols) {
    fprintf(out, "%lu\n", c);
  }
  fclose(out);

  out = openFile("forcedOneCols.txt");
  for (auto c : forced_one_cols) {
    fprintf(out, "%lu\n", c);
  }
  fclose(out);
}

void CalcPairs::record_pair_count(FILE* stream, const std::vector<std::size_t> &count) const {
  for (std::size_t i = 0; i < count.size()-1; ++i) {
    fprintf(stream, "%lu,", count[i]);
  }
  fprintf(stream, "%lu\n", count[count.size()-1]);
}

void CalcPairs::calc_free_row_pairs() {
  FILE *output = openFile("rowPairs.csv");

  std::vector<std::size_t> count;

  for (std::size_t idx = 0; idx < free_rows.size()-1; ++idx) {
    const std::size_t i1 = free_rows[idx];
    const std::size_t num_pairs = free_rows.size()-1-idx;
    count.resize(num_pairs, num_cols);

    for (std::size_t idx2 = idx + 1; idx2 < free_rows.size(); ++idx2) {
      const std::size_t i2 = free_rows[idx2];
     
      for (auto j : free_cols) {
        if (data->is_data_na(i1, j) || data->is_data_na(i2, j)) {
          --count[idx2 - idx - 1];
        }
      }
    }

    record_pair_count(output, count);
    count.clear();
  }
}

void CalcPairs::calc_free_col_pairs() {
  FILE *output = openFile("colPairs.csv");

    std::vector<std::size_t> count;

  for (std::size_t idx = 0; idx < free_cols.size()-1; ++idx) {
    const std::size_t j1 = free_cols[idx];
    const std::size_t num_pairs = free_cols.size()-1-idx;
    count.resize(num_pairs, num_rows);

    for (std::size_t idx2 = idx + 1; idx2 < free_cols.size(); ++idx2) {
      const std::size_t j2 = free_cols[idx2];
     
      for (auto i : free_rows) {
        if (data->is_data_na(i, j1) || data->is_data_na(i, j2)) {
          --count[idx2 - idx - 1];
        }
      }
    }

    record_pair_count(output, count);
    count.clear();
  }
}