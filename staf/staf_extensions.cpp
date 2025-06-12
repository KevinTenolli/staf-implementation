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
std::tuple<std::vector<torch::Tensor>, std::vector<torch::Tensor>,
           std::vector<torch::Tensor>>
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

  suffix_forest forest(nr_tries, score_lambda);
  forest.create_forest(col_pointers, row_indices, n_cols);
  auto binary_csr = forest.build_csr(n_rows);

  std::vector<torch::Tensor> csr_tensors = {
      torch::tensor(binary_csr.get_row_ptr(), torch::kInt32),
      torch::tensor(binary_csr.get_col_indices(), torch::kInt32),
      torch::tensor(binary_csr.get_data(), torch::kFloat32)};

  std::vector<torch::Tensor> map_tensors = {
      torch::tensor(std::get<0>(binary_csr.get_mapped_rows()), torch::kInt32),
      torch::tensor(std::get<1>(binary_csr.get_mapped_rows()), torch::kInt32)};

  std::vector<torch::Tensor> packed_suffix_data = {
      torch::tensor(binary_csr.get_suffix_row_ptr(), torch::kInt32),
      torch::tensor(binary_csr.get_suffix_col_indices(), torch::kInt32),
      torch::tensor(binary_csr.get_suffix_data(), torch::kFloat32)};

  return std::make_tuple(csr_tensors, packed_suffix_data, map_tensors);
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
