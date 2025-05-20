#include "binary_csr.cpp"
#include "suffix_forest.cpp"
#include <cstdint>
#include <omp.h>
#include <torch/extension.h>
#include <vector>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

/*-----------------------Matrix creation--------------------------------*/

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

  int32_t row_indices[] = {
      0, 2, 3, 4, // col 0
      0, 2, 3, 4, // col 1
      1, 4,       // col 2
      1, 4,       // col 3
      0, 2, 3, 4, // col 4
      0, 2, 3, 4  // col 5
  };

  int32_t col_pointers[] = {0, 4, 8, 10, 12, 16, 20};
  auto col_size = 6;
  auto forest = suffix_forest();
  forest.create_forest(col_pointers, row_indices, col_size);
  forest.print_forest();
  // Convert patterns to torch tensors (placeholder - modify as needed)
  std::vector<torch::Tensor> result;
  // ... implementation to convert patterns to tensors ...

  return result;
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
