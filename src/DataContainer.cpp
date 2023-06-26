#include <fstream>
#include <sstream>
#include <algorithm>
#include <assert.h>

#include "DataContainer.h"

DataContainer::DataContainer(const std::string &file_name,
                             const std::string &_na_symbol) :  na_symbol(_na_symbol)
{
  read(file_name);
  calcNumInvalid();
}

DataContainer::~DataContainer()
{}

std::size_t DataContainer::get_num_rows() const {
  return header_rows.size() + header_cols.size();
}

std::size_t DataContainer::get_num_cols() const {
  return header_rows.empty() ? 0 : header_rows[0].size();
}

std::size_t DataContainer::get_num_header_rows() const {
  return header_rows.size();
}

std::size_t DataContainer::get_num_header_cols() const {
  return header_cols.empty() ? 0 : header_cols[0].size();
}

std::size_t DataContainer::get_num_data_rows() const {
  return data.size();
}

std::size_t DataContainer::get_num_data_cols() const {
  return data.empty() ? 0 : data[0].size();
}

std::size_t DataContainer::get_num_data() const {
  return get_num_data_rows() * get_num_data_cols();
}

bool DataContainer::is_data_na(const std::size_t i, const std::size_t j) const {
  assert(i < get_num_data_rows());
  assert(j < get_num_data_cols());
  return data[i][j].compare(na_symbol) == 0;
}


std::string DataContainer::get_data(const std::size_t i, const std::size_t j) const {
  assert(i < get_num_data_rows());
  assert(j < get_num_data_cols());

  return data[i][j];
}

std::size_t DataContainer::get_num_valid_data() const {
  std::size_t count = 0;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
      if (!is_data_na(i, j)) {
        ++count;
      }
    }
  }
  return count;
}

void DataContainer::read(const std::string &file_name,
                         const std::size_t num_header_rows,
                         const std::size_t num_header_cols)
{
  std::string line;
  std::ifstream input;

  input.open(file_name.c_str());
  if (!input)
    throw std::runtime_error("Input file could not be opened.");
  
  // Determine the number of rows
  const std::size_t num_rows = std::count(std::istreambuf_iterator<char>(input),
                                           std::istreambuf_iterator<char>(), '\n');

  // Determine the number of columns
  input.seekg(std::ios_base::beg);  // Go to beginning of file
  std::getline(input, line); // Read in first line
  const std::size_t num_cols = std::count(line.begin(), line.end(), '\t') + 1;

  const std::size_t num_data_rows = num_rows - num_header_rows;
  const std::size_t num_data_cols = num_cols - num_header_cols;


  // Allocate memory
  std::vector<std::string> header_rows_row(num_cols);
  for (std::size_t i = 0; i < num_header_rows; ++i) {
    header_rows.push_back(header_rows_row);
  }

  std::vector<std::string> header_cols_row(num_header_cols);
  for (std::size_t i = 0; i < num_data_rows; ++i) {
    header_cols.push_back(header_cols_row);
  }

  std::vector<std::string> data_row(num_data_cols);
  for (std::size_t i = 0; i < num_data_rows; ++i) {
    data.push_back(data_row);
  }

  // Read in data
  input.seekg(std::ios_base::beg);  // Go to beginning of file
  for (std::size_t i = 0; i < num_header_rows; ++i) {
    std::getline(input, line);

    std::istringstream iss(line);
    std::string token;
    for (std::size_t j = 0; j < num_cols; ++j) {
      std::getline(iss, token, '\t');
      header_rows[i][j] = token;
    }
  }

  for (std::size_t i = 0; i < num_data_rows; ++i) {
    std::getline(input, line);

    std::istringstream iss(line);
    std::string token;
    for (std::size_t j = 0; j < num_header_cols; ++j) {
      std::getline(iss, token, '\t');
      header_cols[i][j] = token;
    }
    for (std::size_t j = 0; j < num_data_cols; ++j) {
      std::getline(iss, token, '\t');
      data[i][j] = token;
    }
  }

  input.close();
}

void DataContainer::write(const std::string &file_name,
                          const std::vector<bool> &rows_to_keep,
                          const std::vector<bool> &cols_to_keep) const
{
  assert(header_rows.size() > 0);
  assert(header_cols.size() > 0);
  assert(data.size() > 0);
  assert(data.size() == rows_to_keep.size());
  assert(data[0].size() == cols_to_keep.size());
  assert(header_cols.size() == data.size());
  assert(header_rows[0].size() == header_cols[0].size() + data[0].size());
  
  const std::size_t num_header_rows = get_num_header_rows();
  const std::size_t num_header_cols = get_num_header_cols();
  const std::size_t num_data_rows = get_num_data_rows();
  const std::size_t num_data_cols = get_num_data_cols();
  
  FILE *output;
  if ((output = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file (%s).\n", file_name.c_str());
    exit(EXIT_FAILURE);
  }

  // Write header rows
  for (std::size_t i = 0; i < num_header_rows; ++i) {
    fprintf(output, "%s", header_rows[i][0].c_str());

    for (std::size_t j = 1; j < num_header_cols; ++j) {
      fprintf(output, "\t%s", header_rows[i][j].c_str());
    }

    for (std::size_t j = 0; j < num_data_cols; ++j) {
      if (cols_to_keep[j]) {
        fprintf(output, "\t%s", header_rows[i][j + num_header_cols].c_str());
      }
    }
    fprintf(output, "\n");
  }

  // Write header cols and data
  for (std::size_t i = 0; i < num_data_rows; ++i) {
    if (rows_to_keep[i]) {
      fprintf(output, "%s", header_cols[i][0].c_str());

      for (std::size_t j = 1; j < num_header_cols; ++j) {
        fprintf(output, "\t%s", header_rows[i][j].c_str());
      }

      for (std::size_t j = 0; j < num_data_cols; ++j) {
        if (cols_to_keep[j]) {
          fprintf(output, "\t%s", data[i][j].c_str());
        }
      }
      fprintf(output, "\n");
    }
  }

  fclose(output);
}

std::size_t DataContainer::get_num_valid_data_kept(const std::vector<bool> &keep_row,
                                                   const std::vector<bool> &keep_col) const
{
  assert(keep_row.size() == get_num_data_rows());
  assert(keep_col.size() == get_num_data_cols());

  std::size_t count = 0;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
      if (keep_row[i] && keep_col[j] && !is_data_na(i, j)) {
        ++count;
      }
    }
  }
  return count; 
}

std::size_t DataContainer::get_num_valid_data_kept(const std::vector<int> &keep_row,
                                                   const std::vector<int> &keep_col) const
{
  assert(keep_row.size() == get_num_data_rows());
  assert(keep_col.size() == get_num_data_cols());

  std::size_t count = 0;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
      if ((keep_row[i] == 1) && (keep_col[j] == 1) && !is_data_na(i, j)) {
        ++count;
      }
    }
  }
  return count; 
}

double DataContainer::get_perc_miss_row(const std::size_t row) const {
  assert(row < get_num_data_rows());

  return static_cast<double>(get_num_invalid_in_row(row)) / get_num_data_rows();
}

double DataContainer::get_perc_miss_col(const std::size_t col) const {
  assert(col < get_num_data_cols());
  
  return static_cast<double>(get_num_invalid_in_col(col)) / get_num_data_cols();
}

double DataContainer::get_max_perc_miss_row() const
{
  double max_perc_miss = 0.0;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    std::size_t count_miss = 0;
    for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
      if (is_data_na(i,j)) {
        ++count_miss;
      }
    }
    double perc_miss = static_cast<double>(count_miss) / get_num_data_cols();
    if (perc_miss > max_perc_miss) {
      max_perc_miss = perc_miss;
    }
  }
  return max_perc_miss;
}

double DataContainer::get_max_perc_miss_col() const
{
  double max_perc_miss = 0.0;
  for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
    std::size_t count_miss = 0;
    for (std::size_t i = 0; i < get_num_data_rows(); ++i) {    
      if (is_data_na(i,j)) {
        ++count_miss;
      }
    }
    double perc_miss = static_cast<double>(count_miss) / get_num_data_rows();
    if (perc_miss > max_perc_miss) {
      max_perc_miss = perc_miss;
    }
  }
  return max_perc_miss;
}

double DataContainer::get_min_perc_miss_row() const
{
  double min_perc_miss = 1.0;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    std::size_t count_miss = 0;
    for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
      if (is_data_na(i,j)) {
        ++count_miss;
      }
    }
    double perc_miss = static_cast<double>(count_miss) / get_num_data_cols();
    if (perc_miss < min_perc_miss) {
      min_perc_miss = perc_miss;
    }
  }
  return min_perc_miss;
}

double DataContainer::get_min_perc_miss_col() const
{
  double min_perc_miss = 1.0;
  for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
    std::size_t count_miss = 0;
    for (std::size_t i = 0; i < get_num_data_rows(); ++i) {    
      if (is_data_na(i,j)) {
        ++count_miss;
      }
    }
    double perc_miss = static_cast<double>(count_miss) / get_num_data_rows();
    if (perc_miss < min_perc_miss) {
      min_perc_miss = perc_miss;
    }
  }
  return min_perc_miss;
}

std::vector<std::size_t> DataContainer::get_idx_of_invalid_row(const std::size_t row) const {
  assert(row < get_num_data_rows());

  std::vector<std::size_t> invalid_idx;
  for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
    if (is_data_na(row, j)) {
      invalid_idx.push_back(j);
    }
  }
  return invalid_idx;
}

std::vector<std::size_t> DataContainer::get_idx_of_invalid_col(const std::size_t col) const {
  assert(col < get_num_data_cols());

  std::vector<std::size_t> invalid_idx;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    if (is_data_na(i, col)) {
      invalid_idx.push_back(i);
    }
  }
  return invalid_idx;
}


std::size_t DataContainer::get_num_invalid_in_row(const std::size_t row) const {
  assert(row < get_num_data_rows());

  // std::size_t count = 0;
  // for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
  //   if (is_data_na(row, j)) {
  //     ++count;
  //   }
  // }
  // return count;
  return nInvalidRow[row];
}

std::size_t DataContainer::get_num_invalid_in_col(const std::size_t col) const {
  assert(col < get_num_data_cols());

  // std::size_t count = 0;
  // for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
  //   if (is_data_na(i, col)) {
  //     ++count;
  //   }
  // }
  // return count;
  return nInvalidCol[col];
}

std::size_t DataContainer::get_num_valid_in_row(const std::size_t row) const {
  assert(row < get_num_data_rows());

  return get_num_data_cols() - get_num_invalid_in_row(row);
}

std::size_t DataContainer::get_num_valid_in_col(const std::size_t col) const {
  assert(col < get_num_data_cols());

  return get_num_data_rows() - get_num_invalid_in_col(col);
}

std::size_t DataContainer::get_num_rows_with_all_valid() const {
  std::size_t count = 0;

  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    bool all_valid = true;
    for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
      if (is_data_na(i,j)) {
        all_valid = false;
        break;
      }
    }
    if (all_valid) {
      ++count;
    }
  }
  return count;
}

std::size_t DataContainer::get_num_cols_with_all_valid() const {
  std::size_t count = 0;

  for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
    bool all_valid = true;
    for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
      if (is_data_na(i,j)) {
        all_valid = false;
        break;
      }
    }
    if (all_valid) {
      ++count;
    }
  }
  return count;
}

void DataContainer::calcNumInvalid() {
  nInvalidRow.resize(get_num_data_rows());
  nInvalidCol.resize(get_num_data_cols());

  for (std::size_t i = 0; i < nInvalidRow.size(); ++i) {
    nInvalidRow[i] = calcNumInvalidRow(i);
  }
  for(std::size_t j = 0; j < nInvalidCol.size(); ++j) {
    nInvalidCol[j] = calcNumInvalidCol(j);
  }
}

std::size_t DataContainer::calcNumInvalidRow(const std::size_t row) const {
  assert(row < get_num_data_rows());

  std::size_t count = 0;
  for (std::size_t j = 0; j < get_num_data_cols(); ++j) {
    if (is_data_na(row, j)) {
      ++count;
    }
  }
  return count;
}

std::size_t DataContainer::calcNumInvalidCol(const std::size_t col) const {
  assert(col < get_num_data_cols());

  std::size_t count = 0;
  for (std::size_t i = 0; i < get_num_data_rows(); ++i) {
    if (is_data_na(i, col)) {
      ++count;
    }
  }
  return count;
}

void DataContainer::printNumMissingRow() const {
  assert(nInvalidRow.size() > 0);

  std::vector<std::pair<std::size_t, std::size_t>> count;
  fprintf(stderr, "nInvalidSize: %lu\n", nInvalidRow.size());

  for (auto nMiss : nInvalidRow) {
    bool found = false;
    for (std::size_t i = 0; i < count.size(); ++i) {
      if (count[i].first == nMiss) {
        ++count[i].second;
        found = true;
        break;
      }
    }
    if (!found) {
      count.push_back(std::make_pair(nMiss, 1));
    }
  }

  for (auto c : count) {
    fprintf(stderr, "%lu, %lu\n", c.first, c.second);
  }
}

void DataContainer::printNumMissingCol() const {
  assert(nInvalidCol.size() > 0);

  std::vector<std::pair<std::size_t, std::size_t>> count;
  fprintf(stderr, "nInvalidSize: %lu\n", nInvalidCol.size());

  for (auto nMiss : nInvalidCol) {
    bool found = false;
    for (std::size_t i = 0; i < count.size(); ++i) {
      if (count[i].first == nMiss) {
        ++count[i].second;
        found = true;
        break;
      }
    }
    if (!found) {
      count.push_back(std::make_pair(nMiss, 1));
    }
  }

  for (auto c : count) {
    fprintf(stderr, "%lu, %lu\n", c.first, c.second);
  }
}