#include "ElementSolverWorker.h"
#include "Parallel.h"

ElementSolverWorker::ElementSolverWorker(const DataContainer &_data,
                                         const std::string &_scratch_dir,
                                         const std::size_t _LARGE_MATRIX) : data(&_data),
                                                                            num_rows(data->get_num_data_rows()),
                                                                            num_cols(data->get_num_data_cols()),
                                                                            scratch_file(_scratch_dir),
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
  std::string file_name = scratch_file + "forcedOneRows.txt";
  FILE* input = open_file_for_read(file_name);
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
  std::string file_name = scratch_file + "forcedOneCols.txt";
  FILE* input = open_file_for_read(file_name);
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
  std::string file_name = scratch_file + "freeRows.txt";
  FILE* input = open_file_for_read(file_name);
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
  std::string file_name = scratch_file + "freeCols.txt";
  FILE* input = open_file_for_read(file_name);
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

  std::vector<std::size_t> pairs;
  std::size_t num_pairs;

  // Loop through each free row & receive row-pairs if it is valid
  for (std::size_t i = 0; i < free_rows.size()-1; ++i) {
    MPI_Recv(&valid_row[i], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (valid_row[i] == 1) {
      MPI_Recv(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (num_pairs > 0) {
        pairs.resize(num_pairs);
        MPI_Recv(&pairs[0], num_pairs, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        row_pairs.push_back(std::make_pair(i, pairs));
        pairs.clear();
      }
    }
  }
  MPI_Recv(&valid_row[free_rows.size()-1], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  
  // Loop through each free column & receive col-pairs if it is valid
  for (std::size_t j = 0; j < free_cols.size()-1; ++j) {
    MPI_Recv(&valid_col[j], 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (valid_col[j] == 1) {
      MPI_Recv(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);      
      
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