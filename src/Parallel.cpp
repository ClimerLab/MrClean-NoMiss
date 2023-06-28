#include "Parallel.h"

//------------------------------------------------------------------------------
// Returns the world_rank
//------------------------------------------------------------------------------
int Parallel::get_world_rank()
{
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  return world_rank;
}


//------------------------------------------------------------------------------
// Returns the world_size
//------------------------------------------------------------------------------
int Parallel::get_world_size()
{
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  return world_size;
}

