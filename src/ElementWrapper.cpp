#include <cstdio>
#include <cstdlib>
#include <string>
#include "DataContainer.h"
#include "Timer.h"
#include "ConfigParser.h"
#include "NoMissSummary.h"
#include "ElementSolverController.h"
#include "ElementSolverWorker.h"
#include "Parallel.h"

int main(int argc, char *argv[]) {
  //*
  //* MPI init
  //*
  MPI_Init(NULL, NULL);
  const int world_rank = Parallel::get_world_rank();
  const int world_size = Parallel::get_world_size();

  try {
    if (world_rank == 0) {
      if (!((argc == 4) || (argc == 6))) {
        fprintf(stderr, "Usage: %s <data_file> <na_symbol> <incumbent_file> (opt)<num_header_rows> (opt)<num_header_cols>\n", argv[0]);
        exit(1);
      } else if (world_size < 2) {
        fprintf(stderr, "world_size must be greater than 1.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
      }
    }
    
    std::string data_file(argv[1]);
    std::string na_symbol(argv[2]);
    std::string incumbent_file(argv[3]);
    std::size_t num_header_rows = 1;
    std::size_t num_header_cols = 1;

    if (argc == 6) {
      num_header_rows = std::stoul(argv[4]);
      num_header_cols = std::stoul(argv[5]);
    }
    DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);
    ConfigParser parser("config.cfg");
    const bool PRINT_SUMMARY = parser.getBool("PRINT_SUMMARY");
    const bool WRITE_STATS = parser.getBool("WRITE_STATS");
    const std::size_t LARGE_MATRIX = parser.getSizeT("LARGE_MATRIX");

    switch (world_rank) {
      case 0: {
        Timer timer;
        // Construct & solve Element problem
        std::size_t num_rows_to_keep = 0, num_cols_to_keep = 0, num_val_elements = 0;
        double run_time = 0.0;
        std::vector<int> rows_to_keep(data.get_num_data_rows(), 0), cols_to_keep(data.get_num_data_cols(), 0);
  
        timer.restart();
        ElementSolverController controller(data, incumbent_file);
        controller.work();

        while (controller.workers_still_working()) {
          controller.wait_for_workers();
        }

        controller.signal_workers_to_end();

        rows_to_keep = controller.get_rows_to_keep();
        cols_to_keep = controller.get_cols_to_keep();
        num_rows_to_keep = controller.get_num_rows_to_keep();
        num_cols_to_keep = controller.get_num_cols_to_keep();
        run_time = timer.elapsed_cpu_time();
        num_val_elements = data.get_num_valid_data_kept(rows_to_keep, cols_to_keep);

        if (PRINT_SUMMARY) {
          noMissSummary::summarize_results(data, na_symbol, "ElementIp", run_time, num_rows_to_keep, num_cols_to_keep, rows_to_keep, cols_to_keep);
        }

        // Wrtie statistics to file
        if (WRITE_STATS) {
          noMissSummary::write_stats_to_file("ElementIp_summary.csv", data_file, run_time, num_val_elements, num_rows_to_keep, num_cols_to_keep);
        }

        break;
      }

      default: {
        ElementSolverWorker worker(data, LARGE_MATRIX);
        while (!worker.end()) {
          worker.work();
        }
        break;
      }
    }
  } catch (std::exception &e) {
    fprintf(stderr, "  *** Fatal error reported by rank_%d: %s *** \n", world_rank, e.what());    
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  MPI_Finalize();
  return 0;
}