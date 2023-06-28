#include "CalcPairsController.h"

#include <assert.h>

#include "Parallel.h"

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
CalcPairsController::CalcPairsController(const DataContainer &_data) :  data(&_data),
                                                    num_rows(data->get_num_data_rows()),
                                                    num_cols(data->get_num_data_cols()),
                                                    world_size(Parallel::get_world_size())
{
  for (std::size_t i = world_size - 1; i > 0; --i) {
    available_workers.push(i);
  }

  calc_free_rows();
  calc_free_cols();
  record_free_rows();
  record_free_cols();
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
CalcPairsController::~CalcPairsController() {}

//------------------------------------------------------------------------------
// Add all rows with no invalid elements to the 'forced_one_rows' vector and
// add all other rows to the 'free_rows' vector.
//------------------------------------------------------------------------------
void CalcPairsController::calc_free_rows() {
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
void CalcPairsController::calc_free_cols() {
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
FILE* CalcPairsController::openFile(const std::string &file_name) const {
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
void CalcPairsController::record_free_rows() const {
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
void CalcPairsController::record_free_cols() const {
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

//------------------------------------------------------------------------------
// Records the 'pair_count' vector using the provided stream
//------------------------------------------------------------------------------
void CalcPairsController::record_pair_count(FILE* stream, const std::vector<std::size_t> &count) const {
  for (std::size_t i = 0; i < count.size()-1; ++i) {
    fprintf(stream, "%lu,", count[i]);
  }
  fprintf(stream, "%lu\n", count[count.size()-1]);
}

void CalcPairsController::send_problem(const int rowCol, const std::size_t idx) {
if (available_workers.empty()) { // wait for a free worker
    receive_completion();
  }

  assert(!available_workers.empty()); // Cannot send problem with no available workers

  const int worker = available_workers.top();

  // Indicate whether a row or column index is being sent
  MPI_Send(&rowCol, 1, MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send the index
  MPI_Send(&idx, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Make the worker unavailable
  available_workers.pop();
  unavailable_workers.insert(worker);
}

void CalcPairsController::receive_completion() {
  assert(available_workers.size() < world_size - 1); // Cannot receive problem when no workers are working

  MPI_Status status;

  // Receive the solution
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  // Receive row/col indicator
  int rowCol;
  MPI_Recv(&rowCol, 1, MPI_INT, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // Receive idx
  std::size_t idx;
  MPI_Recv(&idx, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // Receive number of pairs
  std::size_t nPairs;
  MPI_Recv(&nPairs, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // Receive count for each pair
  std::vector<std::size_t> count(nPairs);
  MPI_Recv(&count[0], nPairs, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);

  record_pair_count(output, count);
  // for (std::size_t i = 0; i < count.size()-1; ++i) {
  //   fprintf(output, "%lu,", count[i]);
  // }
  // fprintf(output, "%lu\n", count[count.size()-1]);

  // Make the workers available again
  available_workers.push(status.MPI_SOURCE);
  unavailable_workers.erase(status.MPI_SOURCE);
}

void CalcPairsController::work() {
  // Calculate values for all pairs of rows and record
  output = openFile("rowPairs.csv");
  for (std::size_t i = 0; i < free_rows.size()-1; ++i) {
    // fprintf(stderr, "Checking row %lu\n", i);
    send_problem(0, i);
  }  
  while (workers_still_working()) {
    receive_completion();
  }
  fclose(output);

  output = openFile("colPairs.csv");
  for (std::size_t j = 0; j < free_cols.size()-1; ++j) {
    send_problem(1, j);
  }
  while (workers_still_working()) {
    receive_completion();
  }
  fclose(output);
}

void CalcPairsController::signal_workers_to_end() {
  char signal = 0;
  for (std::size_t i = 1; i < world_size; ++i) {
    MPI_Send(&signal, 1, MPI_CHAR, i, Parallel::CONVERGE_TAG, MPI_COMM_WORLD);
  }
}

void CalcPairsController::wait_for_workers() {
  receive_completion();
}

bool CalcPairsController::workers_still_working() {
  return (available_workers.size() < (world_size - 1));
}
