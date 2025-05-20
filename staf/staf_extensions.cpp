#include "suffix_forest.hpp"
#include <cstdint>
#include <omp.h>
#include <torch/extension.h>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

/*---------------------------Main function-----------------------------*/
std::vector<torch::Tensor> init_staf_(const torch::Tensor &row_idx,
                                      const torch::Tensor &col_idx,
                                      const torch::Tensor &values,
                                      const size_t n_rows,
                                      const size_t n_cols) {

  CHECK_DTYPE(row_idx, torch::kInt32);
  CHECK_DTYPE(col_idx, torch::kInt32);
  CHECK_DTYPE(values, torch::kFloat32);

  int32_t *array_of_rows = row_idx.data_ptr<int32_t>();
  int32_t *array_of_cols = col_idx.data_ptr<int32_t>();
  float *array_of_values = values.data_ptr<float>();

  // Example static input
  int32_t row_indices[] = {
      0, 2, 3, 4, // col 0
      0, 2, 3, 4, // col 1
      1,          // col 2
      1, 4,       // col 3
      0, 2, 3, 4, // col 4
      0, 2, 3, 4  // col 5
  };

  int32_t col_pointers[] = {0, 4, 8, 9, 11, 15, 19};
  int32_t col_size = 6;

  suffix_forest forest;
  forest.create_forest(col_pointers, row_indices, col_size);
  forest.print_forest();

  // Convert shared patterns to Torch tensors
  std::vector<torch::Tensor> result;

  return result;
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
