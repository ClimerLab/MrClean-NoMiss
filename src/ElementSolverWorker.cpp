#include "ElementSolverWorker.h"
#include "Parallel.h"

ElementSolverWorker::ElementSolverWorker(const DataContainer &_data,
                                         const std::size_t _LARGE_MATRIX) : data(&_data),
                                                                            num_rows(data->get_num_data_rows()),
                                                                            num_cols(data->get_num_data_cols()),
                                                                            world_rank(Parallel::get_world_rank()),
                                                                            LARGE_MATRIX(_LARGE_MATRIX),
                                                                            end_(false),
                                                                            row_sum(0),
                                                                            min_cols(0),
                                                                            valid_row(num_rows, 1),
                                                                            valid_col(num_cols, 1),
                                                                            obj_value(0),
                                                                            rows_to_keep(num_rows, 0),
                                                                            cols_to_keep(num_cols, 0) {
  read_forced_one_rows();
  read_forced_one_cols();
  read_free_rows();
  read_free_cols();
}

ElementSolverWorker::~ElementSolverWorker() { }

void ElementSolverWorker::work() {
  receive_problem();
  if (end_) {return;}

  ElementIpSolver ip_solver(*data,
                            row_sum,
                            min_cols,
                            forced_one_rows,
                            forced_one_cols,
                            free_rows,
                            free_cols,
                            LARGE_MATRIX);
  
  // Add row constraints based on row_pairs
  for (std::size_t i = 0; i < free_rows.size()-1; ++i) {
    if (valid_row[i] == 0) {
      ip_solver.set_row_to_zero(i);
    }
  }
  for (auto pair : row_pairs) {
    ip_solver.add_row_pairs_cut(pair.first, pair.second);
  }

  // Add columns constraints based on col_pairs
  for (std::size_t j = 0; j < free_cols.size()-1; ++j) {
    if (valid_col[j] == 0) {
      ip_solver.set_col_to_zero(j);
    }
  }
  for (auto pair : col_pairs) {
    ip_solver.add_col_pairs_cut(pair.first, pair.second);
  }

  clear_pairs();

  ip_solver.solve();
  obj_value = ip_solver.get_obj_value();

  if (obj_value >= min_cols) {
    rows_to_keep = ip_solver.get_rows_to_keep();
    cols_to_keep = ip_solver.get_cols_to_keep();
  }

  send_back_solution();
}

bool ElementSolverWorker::end() const {
  return end_;
}

FILE* ElementSolverWorker::open_file_for_read(const std::string &file_name) const {
  FILE* input;
  if ((input = fopen(file_name.c_str(), "r")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file %s\n", file_name.c_str());
    exit(1);
  }
  return input;
}

void ElementSolverWorker::read_forced_one_rows() {
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

void ElementSolverWorker::read_forced_one_cols() {
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

void ElementSolverWorker::read_free_rows() {
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

void ElementSolverWorker::read_free_cols() {
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

void ElementSolverWorker::receive_problem() {
  MPI_Status status;

  // Check if received a signal to end
  MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  if (status.MPI_TAG == Parallel::CONVERGE_TAG) {
    char signal;
    MPI_Recv(&signal, 1, MPI_CHAR, 0, Parallel::CONVERGE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    end_ = true;
    
    #ifndef NDEBUG
      fprintf(stderr, "Rank %lu received signal to end\n", world_rank);
    #endif

    return;
  }

  // Receive whether the idx is for a row or column
  MPI_Recv(&row_sum, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Receive the index
  MPI_Recv(&min_cols, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Receive valid rows  
  // MPI_Recv(&valid_row[0], valid_row.size(), CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Receive valid cols
  // MPI_Recv(&valid_col[0], valid_col.size(), CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  std::vector<std::size_t> pairs;
  std::size_t num_pairs;

  // fprintf(stderr, "Received row_sum and min_cols\n");
  // Loop through each free row & receive row-pairs if it is valid
  for (std::size_t i = 0; i < free_rows.size()-1; ++i) {
    MPI_Recv(&valid_row[i], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // fprintf(stderr, "Row %lu is %d\n", i, valid_row[i]);
    if (valid_row[i] == 1) {
      MPI_Recv(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // fprintf(stderr, "Row %lu has %lu pairs below thershold\n", i, num_pairs);
      if (num_pairs > 0) {
        pairs.resize(num_pairs);
        MPI_Recv(&pairs[0], num_pairs, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (auto p : pairs) {
          if (p >= num_rows) {
            fprintf(stderr, "ERROR - Trying to send row pair (%lu, %lu)\n", i, p);
            exit(1);
          }
        }
        row_pairs.push_back(std::make_pair(i, pairs));
        pairs.clear();
      }

      
    }
  }
  MPI_Recv(&valid_row[free_rows.size()-1], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  // fprintf(stderr, "Row %lu is %d\n", free_rows.size()-1, valid_row[free_rows.size()-1]);
  // fprintf(stderr, "Received row_pairs\n");

  // Loop through each free column & receive col-pairs if it is valid
  for (std::size_t j = 0; j < free_cols.size()-1; ++j) {
    MPI_Recv(&valid_col[j], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (valid_col[j] == 1) {
      MPI_Recv(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);      
      
      // fprintf(stderr, "Receiving %lu pairs for col %lu\n", num_pairs, j);

      if (num_pairs >= free_cols.size()) {
        fprintf(stderr, "Receving %lu pairs\n", num_pairs);
        exit(1);
      }

      if (num_pairs > 0) {
        pairs.resize(num_pairs);
        MPI_Recv(&pairs[0], num_pairs, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (auto p : pairs) {
          if (p >= num_cols) {
            fprintf(stderr, "ERROR - Trying to send col pair (%lu, %lu)\n", j, p);
            exit(1);
          }
        }
        col_pairs.push_back(std::make_pair(j,pairs));
        pairs.clear();
      }      
    }
  }
  MPI_Recv(&valid_col[free_cols.size()-1], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  // fprintf(stderr, "Col %lu is %d\n", free_cols.size()-1, valid_col[free_cols.size()-1]);
  // fprintf(stderr, "Received col_pairs\n");
  // // Loop through each free column & send col-pairs if it is valid
  // for (std::size_t j = 0; j < free_cols.size()-1; ++j) {
  //   MPI_Send(&valid_col[j], 1, MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  //   if (valid_col[j] == 1) {
  //     auto cp = row_pairs.getPairsLtThresh(j, min_cols, valid_col);
  //     std::size_t num_pairs = cp.size();
  //     MPI_Send(&num_pairs, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  //     if (num_pairs > 0) {
  //       MPI_Send(&cp[0], cp.size(), CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  //     }
  //   }
  // }
  // MPI_Send(&valid_col[free_cols.size()-1], 1, MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  // // Add row constraints based on row_pairs
  // for (std::size_t i = 0; i < free_rows.size(); ++i) {    
  //   MPI_Recv(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //   pairs.resize(num_pairs);
  //   if (num_pairs > 0) {
  //     MPI_Recv(&pairs[0], num_pairs, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //   }
  //   if (valid_row[i]) {
  //     if (!pairs.empty()) {
  //       ip_solver.add_row_pairs_cut(i, pairs);
  //     }
  //   } else {
  //     ip_solver.set_row_to_zero(i);
  //   }
  // }
  // // Add columns constraints based on col_pairs
  // for (std::size_t j = 0; j < free_cols.size(); ++j) {
  //   MPI_Recv(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //   pairs.resize(num_pairs);
  //   if (num_pairs > 0) {
  //     MPI_Recv(&pairs[0], num_pairs, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  //   }
  //   if (valid_col[j]) {
  //     if (!pairs.empty()) {
  //       ip_solver.add_col_pairs_cut(j, pairs);
  //     }
  //   } else {
  //     ip_solver.set_col_to_zero(j);
  //   }
  // }
}

void ElementSolverWorker::send_back_solution() {
  // Send row/col indicator  
  MPI_Ssend(&row_sum, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send index
  MPI_Ssend(&obj_value, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  if (obj_value >= min_cols) {
    MPI_Ssend(&rows_to_keep[0], num_rows, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
    MPI_Ssend(&cols_to_keep[0], num_cols, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
  }
}

void ElementSolverWorker::clear_pairs() {
  for (auto p : row_pairs) {
    p.second.clear();
  }
  row_pairs.clear();

  for (auto p : col_pairs) {
    p.second.clear();
  }
  col_pairs.clear();
}