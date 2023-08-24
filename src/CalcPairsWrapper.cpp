#include <stdio.h>
#include <string>

#include "DataContainer.h"
#include "Timer.h"
#include "CalcPairsController.h"
#include "CalcPairsWorker.h"
#include "Parallel.h"

int main(int argc, char* argv[]) {
  //*
  //* MPI init
  //*
  MPI_Init(NULL, NULL);
  const int world_rank = Parallel::get_world_rank();
  const int world_size = Parallel::get_world_size();

  try {
    if (world_rank == 0) {
      if (!((argc == 3) || (argc == 5))) {
        fprintf(stderr, "Usage: %s <data_file> <na_symbol>\n", argv[0]);
        exit(1);
      } else if (world_size < 2) {
        fprintf(stderr, "world_size must be greater than 1.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
      }
    }
    
    std::string data_file(argv[1]);
    std::string na_symbol(argv[2]);
    std::size_t num_header_rows = 1;
    std::size_t num_header_cols = 1;

    if (argc == 5) {
      num_header_rows = std::stoul(argv[3]);
      num_header_cols = std::stoul(argv[4]);
    }
    DataContainer data(data_file, na_symbol, num_header_rows, num_header_cols);

    switch (world_rank) {
      case 0: {
        Timer timer;
        timer.start();
        CalcPairsController controller(data);

        controller.work();

        controller.signal_workers_to_end();
        while (controller.workers_still_working()) {
          controller.wait_for_workers();
        }

        timer.stop();

        std::string file_name = "CalcPairs.csv";
        FILE* output;
        if((output = fopen(file_name.c_str(), "a+")) == nullptr) {
          fprintf(output, "ERROR - Could not open file (%s)\n", file_name.c_str());
          exit(1);
        }
        fprintf(output, "%s,%lf\n", data_file.c_str(), timer.elapsed_cpu_time());
        fclose(output);

        fprintf(stderr, "Summary of CalcPairs\n");
        fprintf(stderr, "\tTook %lf seconds\n\n", timer.elapsed_cpu_time());
        
        break;
      }

      default: {
        CalcPairsWorker worker(data);
        // while (!worker.end()) {
          worker.work();
        // }
        // fprintf(stderr, "Worker has ended\n");
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