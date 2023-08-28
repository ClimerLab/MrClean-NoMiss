#include "CalcPairsController.h"

#include <fstream>

#include <assert.h>

#include "Parallel.h"
#include "CalcPairsCore.h"

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
CalcPairsController::CalcPairsController(const DataContainer &_data,
                                         const std::string &_scratch_dir) : data(&_data),
                                                                            num_rows(data->get_num_data_rows()),
                                                                            num_cols(data->get_num_data_cols()),
                                                                            scratch_dir(_scratch_dir),
                                                                            world_size(Parallel::get_world_size()) {
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
FILE* CalcPairsController::open_file(const std::string &file_name) const {
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
  std::string file_name = scratch_dir + "freeRows.txt";
  FILE* out = open_file(file_name);
  for (auto r : free_rows) {
    fprintf(out, "%lu\n", r);
  }
  fclose(out);
  
  file_name = scratch_dir + "forcedOneRows.txt";
  out = open_file(file_name);
  for (auto r : forced_one_rows) {
    fprintf(out, "%lu\n", r);
  }
  fclose(out);
}

//------------------------------------------------------------------------------
// Records the 'free cols' and 'cols forced to 1' in seperate files.
//------------------------------------------------------------------------------
void CalcPairsController::record_free_cols() const {
  std::string file_name = scratch_dir + "freeCols.txt";
  FILE* out = open_file(file_name);
  for (auto c : free_cols) {
    fprintf(out, "%lu\n", c);
  }
  fclose(out);

  file_name = scratch_dir + "forcedOneCols.txt";
  out = open_file(file_name);
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
  fprintf(stderr, "Sending problem to rank %d\n", worker);

  // Indicate whether a row or column index is being sent
  MPI_Send(&rowCol, 1, MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Send the index
  MPI_Send(&idx, 1, CUSTOM_SIZE_T, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);

  // Make the worker unavailable
  available_workers.pop();
  unavailable_workers.insert(worker);
}

void CalcPairsController::send_start() {
  int start = 1;
  // MPI_Bcast(&start, 1, MPI_INT, 0, MPI_COMM_WORLD);

  while (!available_workers.empty()) {
    const int worker = available_workers.top();
    MPI_Send(&start, 1, MPI_INT, worker, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
    // Make the worker unavailable
    available_workers.pop();
    unavailable_workers.insert(worker);
  }
}

void CalcPairsController::receive_completion() {
  assert(available_workers.size() < world_size - 1); // Cannot receive problem when no workers are working

  MPI_Status status;
  
  MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  int flag;
  MPI_Recv(&flag, 1, MPI_INT, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);

  if (flag != 1) {
    fprintf(stderr, "Received inknown flag\n");
  }

  available_workers.push(status.MPI_SOURCE);
  unavailable_workers.erase(status.MPI_SOURCE);
  // // Receive the solution
  
  // fprintf(stderr, "Rank %d receive solution from %d\n", 0, status.MPI_SOURCE);

  // // Receive row/col indicator
  // int rowCol;
  // MPI_Recv(&rowCol, 1, MPI_INT, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // // Receive idx
  // std::size_t idx;
  // MPI_Recv(&idx, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // // Receive number of pairs
  // std::size_t nPairs;
  // MPI_Recv(&nPairs, 1, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);
  
  // // Receive count for each pair
  // std::vector<std::size_t> count(nPairs);
  // MPI_Recv(&count[0], nPairs, CUSTOM_SIZE_T, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);

  // record_pair_count(output, count);
  
  // // Make the workers available again
  // available_workers.push(status.MPI_SOURCE);
  // unavailable_workers.erase(status.MPI_SOURCE);
  // fprintf(stderr, "%lu available workers\n", available_workers.size());

  // int flag;
  // MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
  // if (flag) {
  //   fprintf(stderr, "Another messsage in buuffer\n");
  // }
}

void CalcPairsController::work() {
  // Test workeres to start once free_row and free_col have been recorded
  send_start();

  // Create local core and calculare alloted pairs
  CalcPairsCore core(*data, scratch_dir, free_rows, free_cols);
  core.work();

  // Wait for all workes to finish
  while (workers_still_working()) {
    receive_completion();
  }
  
  // Combine the various row_pairs file
  combine_row_pair_files();

  // Combine the various col_pairs file
  combine_col_pair_files();
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


void CalcPairsController::combine_row_pair_files() {
  std::string file_name = scratch_dir + "rowPairs.csv";
  std::ofstream output(file_name, std::ios_base::binary);

  for (std::size_t p = 0; p < world_size; ++p) {
    std::string input_file = scratch_dir + "rowPairs_part" + std::to_string(p) + ".csv";
    std::ifstream input(input_file.c_str(), std::ios_base::binary);

    output << input.rdbuf();
  }
}

void CalcPairsController::combine_col_pair_files() {
  std::string file_name = scratch_dir + "colPairs.csv";
  std::ofstream output(file_name, std::ios_base::binary);

  for (std::size_t p = 0; p < world_size; ++p) {
    std::string input_file = scratch_dir + "colPairs_part" + std::to_string(p) + ".csv";
    std::ifstream input(input_file.c_str(), std::ios_base::binary);

    output << input.rdbuf();
  }
}