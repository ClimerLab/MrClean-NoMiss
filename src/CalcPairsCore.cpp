#include "CalcPairsCore.h"
#include "Parallel.h"

CalcPairsCore::CalcPairsCore(const DataContainer &_data,
                             const std::vector<std::size_t> &_free_rows,
                             const std::vector<std::size_t> &_free_cols) :  data(&_data),
                                                                            num_rows(data->get_num_data_rows()),
                                                                            num_cols(data->get_num_data_cols()),
                                                                            world_rank(Parallel::get_world_rank()),
                                                                            world_size(Parallel::get_world_size())
{
}

CalcPairsCore::~CalcPairsCore()
{
}

void CalcPairsCore::work() {
  
}