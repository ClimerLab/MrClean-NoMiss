#ifndef DATA_CONTAINER_H
#define DATA_CONTAINER_H

#include <vector>
#include <string>

class DataContainer
{
private:
  const std::string na_symbol;
  std::vector<std::vector<std::string>> header_rows;
  std::vector<std::vector<std::string>> header_cols;
  std::vector<std::vector<std::string>> data;
  std::vector<std::size_t> nInvalidRow;
  std::vector<std::size_t> nInvalidCol;

  void read(const std::string &file_name, const std::size_t num_header_rows = 1, const std::size_t num_header_cols = 1);
  void calcNumInvalid();
  std::size_t calcNumInvalidRow(const std::size_t row) const;
  std::size_t calcNumInvalidCol(const std::size_t col) const;

public:
  DataContainer(const std::string &file_name, const std::string &_na_symbol);
  ~DataContainer();

  std::size_t get_num_rows() const;
  std::size_t get_num_cols() const;
  std::size_t get_num_header_rows() const;
  std::size_t get_num_header_cols() const;
  std::size_t get_num_data_rows() const;
  std::size_t get_num_data_cols() const;
  std::size_t get_num_data() const;
  
  bool is_data_na(const std::size_t i, const std::size_t j) const;
  std::string get_data(const std::size_t i, const std::size_t j) const;

  std::size_t get_num_valid_data() const;
  std::size_t get_num_valid_data_kept(const std::vector<bool> &keep_row,
                                      const std::vector<bool> &keep_col) const;
  std::size_t get_num_valid_data_kept(const std::vector<int> &keep_row,
                                      const std::vector<int> &keep_col) const;

  void write(const std::string &file_name,
             const std::vector<bool> &rows_to_keep,
             const std::vector<bool> &cols_to_keep) const;
  
  double get_perc_miss_row(const std::size_t row) const;
  double get_perc_miss_col(const std::size_t col) const;
  double get_max_perc_miss_row() const;
  double get_max_perc_miss_col() const;
  double get_min_perc_miss_row() const;
  double get_min_perc_miss_col() const;  

  std::vector<std::size_t> get_idx_of_invalid_row(const std::size_t row) const;
  std::vector<std::size_t> get_idx_of_invalid_col(const std::size_t col) const;

  std::size_t get_num_invalid_in_row(const std::size_t row) const;
  std::size_t get_num_invalid_in_col(const std::size_t col) const;
  std::size_t get_num_valid_in_row(const std::size_t row) const;
  std::size_t get_num_valid_in_col(const std::size_t col) const;

  std::size_t get_num_rows_with_all_valid() const;
  std::size_t get_num_cols_with_all_valid() const;

  void printNumMissingRow() const;
  void printNumMissingCol() const;
};



#endif