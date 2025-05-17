#include "binary_csr.cpp"
#include "suffix_trie.cpp"
#include <cstdint>
#include <omp.h>
#include <torch/extension.h>
#include <vector>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

/*-----------------------Matrix creation--------------------------------*/

void print_dense_matrix(const binary_csr &csr) {
  const std::vector<int> &row_ptr = csr.get_row_ptr();
  const std::vector<int> &col_indices = csr.get_col_indices();
  const std::vector<float> &data = csr.get_data();

  int num_rows = row_ptr.size() - 1;
  int num_cols = 0;

  // Find number of columns
  for (int col : col_indices) {
    if (col + 1 > num_cols) {
      num_cols = col + 1;
    }
  }

  for (int row = 0; row < num_rows; ++row) {
    std::vector<float> dense_row(num_cols, 0.0f);
    int start = row_ptr[row];
    int end = row_ptr[row + 1];

    for (int idx = start; idx < end; ++idx) {
      dense_row[col_indices[idx]] = data[idx];
    }

    for (float val : dense_row) {
      std::cout << val << " ";
    }
    std::cout << "\n";
  }
}

std::vector<std::vector<float>> apply_shared_patterns_to_dense(
    const binary_csr &csr,
    const std::map<std::vector<int>, std::vector<int>> &shared_patterns,
    int no_main_rows, int no_columns) {
  const std::vector<int> &row_ptr = csr.get_row_ptr();
  const std::vector<int> &col_indices = csr.get_col_indices();
  const std::vector<float> &data = csr.get_data();

  // Step 1: Initialize matrix with no_main_rows
  std::vector<std::vector<float>> dense(no_main_rows,
                                        std::vector<float>(no_columns, 0.0f));
  // Step 2: Apply normal (main) rows
  for (int row = 0; row < no_main_rows; ++row) {
    for (int idx = row_ptr[row]; idx < row_ptr[row + 1]; ++idx) {
      dense[row][col_indices[idx]] = data[idx];
    }
  }

  // Step 3: Apply shared patterns (extra rows start after main rows)
  int shared_row_idx = no_main_rows;
  for (const auto &entry : shared_patterns) {
    const std::vector<int> &target_rows = entry.first;

    int start = row_ptr[shared_row_idx];
    int end = row_ptr[shared_row_idx + 1];
    std::vector<int> shared_cols(col_indices.begin() + start,
                                 col_indices.begin() + end);

    for (int row : target_rows) {
      for (int col : shared_cols) {
        dense[row][col] = 1.0f; // overwrite or OR-style merge
      }
    }

    ++shared_row_idx;
  }

  return dense;
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

  /* int32_t rows[] = {0, 0, 0, 1, 1, 1, 1, 2, 2}; */
  /* int32_t columns[] = {0, 1, 2, 0, 1, 2, 3, 0, 2}; */

  int32_t rows[] = {0, 0, 0, 1, 1, 1, 1, 2, 2};
  int32_t columns[] = {4, 5, 6, 4, 5, 6, 7, 4, 6};
  auto trie2 = suffix_trie(8);
  trie2.build_trie_from_tensors(rows, columns, 9);

  // Get all patterns from the trie
  auto shared = trie2.get_shared_patterns();
  auto unique = trie2.get_unique_patterns();

  // print full trie
  trie2.print_trie();

  // create sparse matrix
  auto csr = binary_csr(unique, shared, 3);
  csr.print();
  print_dense_matrix(csr);

  // Convert patterns to torch tensors (placeholder - modify as needed)
  std::vector<torch::Tensor> result;
  // ... implementation to convert patterns to tensors ...

  return result;
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
