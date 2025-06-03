#include "binary_csr.hpp"
#include "suffix_forest.hpp"
#include <cstdint>
#include <iostream>
#include <omp.h>
#include <torch/extension.h>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

/*---------------------------Main function-----------------------------*/
std::tuple<std::vector<torch::Tensor>, std::vector<std::vector<int>>>
init_staf_(const torch::Tensor &col_ptr, const torch::Tensor &row_idx,
           const torch::Tensor &values, const size_t n_rows,
           const size_t n_cols, const size_t score_lambda,
           const size_t nr_tries) {

  CHECK_DTYPE(col_ptr, torch::kInt32);
  CHECK_DTYPE(row_idx, torch::kInt32);
  CHECK_DTYPE(values, torch::kFloat32);

  int32_t *col_pointers = col_ptr.data_ptr<int32_t>();
  int32_t *row_indices = row_idx.data_ptr<int32_t>();
  float *array_of_values = values.data_ptr<float>();

  // Example static input
  /* int32_t row_indices[] = { */
  /*     0, 2, 3, 4, // col 0 */
  /*     0, 2, 3, 4, // col 1 */
  /*     1,          // col 2 */
  /*     1, 4,       // col 3 */
  /*     0, 2, 3, 4, // col 4 */
  /*     0, 2, 3, 4  // col 5 */
  /* }; */
  /* int32_t col_pointers[] = {0, 4, 8, 9, 11, 15, 19}; */
  /* int32_t col_size = 6; */

  suffix_forest forest(nr_tries, score_lambda);
  forest.create_forest(col_pointers, row_indices, n_cols);
  forest.print_forest();
  auto binary_csr = forest.build_csr(n_rows);

  std::vector<torch::Tensor> csr_tensors;

  auto row_ptr_vec = binary_csr.get_row_ptr();
  csr_tensors.push_back(torch::tensor(row_ptr_vec, torch::kInt32));

  auto col_indices_vec = binary_csr.get_col_indices();
  csr_tensors.push_back(torch::tensor(col_indices_vec, torch::kInt32));

  auto data_vec = binary_csr.get_data();
  csr_tensors.push_back(torch::tensor(data_vec, torch::kFloat32));

  return std::make_tuple(csr_tensors, binary_csr.get_mapped_rows());
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
