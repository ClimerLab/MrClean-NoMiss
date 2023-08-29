#include "CalcPairsCore.h"
#include "Parallel.h"

CalcPairsCore::CalcPairsCore(const DataContainer &_data,
                             const std::string &_scratch_dir,
                             const std::vector<std::size_t> &_free_rows,
                             const std::vector<std::size_t> &_free_cols) :  data(&_data),
                                                                            num_rows(data->get_num_data_rows()),
                                                                            num_cols(data->get_num_data_cols()),
                                                                            scratch_dir(_scratch_dir),
                                                                            world_rank(Parallel::get_world_rank()),
                                                                            world_size(Parallel::get_world_size()),
                                                                            free_rows(_free_rows),
                                                                            free_cols(_free_cols)
{}

CalcPairsCore::~CalcPairsCore() {}

void CalcPairsCore::work() {
  std::string file_name = scratch_dir + "rowPairs_part" + std::to_string(world_rank) + ".csv";
  open_file(file_name);

  for (std::size_t idx = world_rank; idx < free_rows.size()-1; idx+=world_size) {
    const std::size_t i1 = free_rows[idx];
    const std::size_t num_pairs = free_rows.size()-1-idx;
    std::vector<std::size_t> count(num_pairs, num_cols);

    std::vector<std::size_t> valid_cols;
    for (auto j : free_cols) {
      if (!data->is_data_na(i1, j)) {
        valid_cols.push_back(j);
      }
    }

    for (std::size_t idx2 = idx + 1; idx2 < free_rows.size(); ++idx2) {
      count[idx2 - idx - 1] -= (free_cols.size() - valid_cols.size());
      const std::size_t i2 = free_rows[idx2];
     
      for (auto j : valid_cols) {
        if (data->is_data_na(i2, j)) {
          --count[idx2 - idx - 1];
        }
      }
    }

    record_pair_count(count, output);
  }

  close_file();


  std::string col_file_name = scratch_dir + "colPairs_part" + std::to_string(world_rank) + ".csv";
  open_file(col_file_name);

  for (std::size_t idx = world_rank; idx < free_cols.size()-1; idx+=world_size) {
    const std::size_t j1 = free_cols[idx];
    const std::size_t num_pairs = free_cols.size()-1-idx;
    std::vector<std::size_t> count(num_pairs, num_rows);

    std::vector<std::size_t> valid_rows;
    for (auto i : free_rows) {
      if (!data->is_data_na(i, j1)) {
        valid_rows.push_back(i);
      }
    }

    for (std::size_t idx2 = idx + 1; idx2 < free_cols.size(); ++idx2) {
      count[idx2 - idx - 1] -= (free_rows.size() - valid_rows.size());
      const std::size_t j2 = free_cols[idx2];
     
      for (auto i : valid_rows) {
        if (data->is_data_na(i, j2)) {
          --count[idx2 - idx - 1];
        }
      }
    }

    record_pair_count(count, output);
  }
  close_file();
}


void CalcPairsCore::open_file(const std::string &file_name) {
  if ((output = fopen(file_name.c_str(), "w")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file %s\n", file_name.c_str());
    exit(1);
  }
}

void CalcPairsCore::close_file() {
  fclose(output);
}

void CalcPairsCore::record_pair_count(const std::vector<std::size_t> &count, FILE *stream) const {
  for (std::size_t i = 0; i < count.size()-1; ++i) {
    fprintf(output, "%lu,", count[i]);
  }
  fprintf(output, "%lu\n", count[count.size()-1]);
}