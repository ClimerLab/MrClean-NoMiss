#ifndef PARALLEL_H
#define PARALLEL_H

#include <limits.h>
//#include <mpi.h>
#include </cluster/spack-2022/opt/spack/linux-centos7-x86_64/gcc-9.3.0/openmpi-4.1.1-udg7sdl3kjslokkcsrmuzz5kn6krohpa/include/mpi.h>
#include <stdint.h>

// https://stackoverflow.com/a/40808411
#if SIZE_MAX == UCHAR_MAX
  #define CUSTOM_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
  #define CUSTOM_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
  #define CUSTOM_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
  #define CUSTOM_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
  #define CUSTOM_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
  #error "Could not find proper substitution for CUSTOM_SIZE_T"
#endif


namespace Parallel
{
  const int SPARSE_TAG = 0;
  const int CONVERGE_TAG = 1;

  int get_world_rank();
  int get_world_size();
}


#endif

