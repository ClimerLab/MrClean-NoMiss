#include "CalcPairsWorker.h"
#include "Parallel.h"
#include "CalcPairsCore.h"

CalcPairsWorker::CalcPairsWorker(const DataContainer &_data,
                                 const std::string &_scratch_dir) : data(&_data),
                                                                    num_rows(data->get_num_data_rows()),
                                                                    num_cols(data->get_num_data_cols()),
                                                                    scratch_dir(_scratch_dir),
                                                                    world_rank(Parallel::get_world_rank()) {}

CalcPairsWorker::~CalcPairsWorker() {}

void CalcPairsWorker::work() {
  receive_start();

  read_free_rows();
  read_free_cols();

  CalcPairsCore core(*data, scratch_dir, free_rows, free_cols);
  core.work();

  send_completion();
}

bool CalcPairsWorker::end() const {
  return end_;
}

FILE* CalcPairsWorker::open_file_for_read(const std::string &file_name) const {
  FILE* input;
  if ((input = fopen(file_name.c_str(), "r")) == nullptr) {
    fprintf(stderr, "ERROR - Could not open file %s\n", file_name.c_str());
    exit(1);
  }
  return input;
}

void CalcPairsWorker::read_free_rows() {
  std::string file_name = scratch_dir + "freeRows.txt";
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

void CalcPairsWorker::read_free_cols() {
  std::string file_name = scratch_dir + "freeCols.txt";
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

void CalcPairsWorker::send_completion() {
  const int status = 1;

  MPI_Send(&status, 1, MPI_INT, 0, Parallel::SPARSE_TAG, MPI_COMM_WORLD);
}

void CalcPairsWorker::receive_start() {
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

  int start;
  MPI_Recv(&start, 1, MPI_INT, status.MPI_SOURCE, Parallel::SPARSE_TAG, MPI_COMM_WORLD, &status);  
}