#include "binary_csr.cpp"
#include "suffix_trie.cpp"
#include <omp.h>
#include <torch/extension.h>
#include <vector>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

/*-----------------------Matrix creation--------------------------------*/
std::vector<std::vector<float>> create_matrix(int32_t *array_of_rows,
                                              int32_t *array_of_cols,
                                              float *array_of_values,
                                              int n_rows, int n_cols,
                                              float initial_value = 0.0f) {
  std::vector<std::vector<float>> matrix(
      n_rows, std::vector<float>(n_cols, initial_value));
  for (int i = 0; i < n_rows; i++) {
    int row = array_of_rows[i];
    int col = array_of_cols[i];
    matrix[row][col] = array_of_values[i];
  }
  return matrix;
}
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

  auto base_matrix = create_matrix(array_of_rows, array_of_cols,
                                   array_of_values, n_rows, n_cols);

  /* auto trie = suffix_trie(3); */
  /**/
  /* // Add all rows to the trie (in reverse order for suffix trie) */
  /* trie.add_matrix_row({1, 0, 1}, 0); */
  /* trie.add_matrix_row({1, 1, 1}, 1); */
  /* trie.add_matrix_row({0, 1, 1}, 2); */
  /* auto patterns = trie.get_shared_patterns(); */
  /* trie.print_trie(); */
  // Get all patterns from the trie
  /* auto patterns = trie.get_shared_patterns(); */
  /**/
  // Create trie with known vector size (n_cols)
  auto trie2 = suffix_trie(3);
  trie2.add_matrix_row({1, 1, 1}, 1);
  trie2.add_matrix_row({1, 1, 1}, 2);
  trie2.add_matrix_row({1, 0, 1}, 3);

  // Get all patterns from the trie
  auto shared = trie2.get_shared_patterns();
  auto unique = trie2.get_unique_patterns();

  // print full trie
  trie2.print_trie();

  // create sparse matrix for the unique paths (main matrix)
  auto csr_unique = binary_csr(unique);
  csr_unique.print();

  // create sparse matrix for the shared paths (rows to be added after matrix
  // operations)
  auto csr_shared = binary_csr(shared);
  csr_shared.print();

  // Convert patterns to torch tensors (placeholder - modify as needed)
  std::vector<torch::Tensor> result;
  // ... implementation to convert patterns to tensors ...

  return result;
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
