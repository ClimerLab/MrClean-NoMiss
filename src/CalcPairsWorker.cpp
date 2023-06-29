#include "CalcPairsWorker.h"
#include "Parallel.h"

CalcPairsWorker::CalcPairsWorker(const DataContainer &_data) :  data(&_data),
                                                                num_rows(data->get_num_data_rows()),
                                                                num_cols(data->get_num_data_cols()),
                                                                world_rank(Parallel::get_world_rank()) {
  calc_free_rows();
  calc_free_cols();
}

CalcPairsWorker::~CalcPairsWorker() {}

//------------------------------------------------------------------------------
// Add all rows with no invalid elements to the 'forced_one_rows' vector and
// add all other rows to the 'free_rows' vector.
//------------------------------------------------------------------------------
void CalcPairsWorker::calc_free_rows() {
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
void CalcPairsWorker::calc_free_cols() {
  for (std::size_t j = 0; j < num_cols; ++j) {
    if (data->get_num_invalid_in_col(j) == 0) {
      forced_one_cols.push_back(j);
    } else {
      free_cols.push_back(j);
    }
  }
}

void CalcPairsWorker::receive_problem() {
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

  fprintf(stderr, "Rank %lu receiving problem\n", world_rank);
  
  // Receive whether the idx is for a row or column
  MPI_Recv(&row_col, 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Receive the index
  MPI_Recv(&idx, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void CalcPairsWorker::send_back_solution() {
  fprintf(stderr, "Rank %lu sending solution\n", world_rank);
  
  // Send row/col indicator  
  MPI_Send(&row_col, 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send index
  MPI_Send(&idx, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send number of pairs
  const std::size_t num_pairs = count.size();
  MPI_Send(&num_pairs, 1, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send count for each pair
  MPI_Send(&count[0], num_pairs, CUSTOM_SIZE_T, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
}

void CalcPairsWorker::work() {
  receive_problem();

  if (row_col == 0) {
    const std::size_t i1 = free_rows[idx];
    std::size_t num_pairs = free_rows.size()-1 - idx;
    count.resize(num_pairs, num_cols);

    for (std::size_t idx2 = idx + 1; idx2 < free_rows.size(); ++idx2) {
      const std::size_t i2 = free_rows[idx2];
     
      for (auto j : free_cols) {
        if (data->is_data_na(i1, j) || data->is_data_na(i2, j)) {
          --count[idx2 - idx - 1];
        }
      }
    }
  } else if (row_col == 1) {
    const std::size_t j1 = free_cols[idx];
    std::size_t num_pairs = free_cols.size()-1 - idx;
    count.resize(num_pairs, num_rows);

    for (std::size_t idx2 = idx + 1; idx2 < free_cols.size(); ++idx2) {
      const std::size_t j2 = free_cols[idx2];
      
      for (auto i : free_rows) {
        if (data->is_data_na(i, j1) || data->is_data_na(i, j2)) {
          --count[idx2 - idx - 1];
        }
      }
    }
  } else {
    fprintf(stderr, "Unknwon rowCol type\n");
    exit(1);
  }

  send_back_solution();
  
  count.clear();
}

bool CalcPairsWorker::end() const {
  return end_;
}


