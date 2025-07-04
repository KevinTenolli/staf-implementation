#include "binary_csr.hpp"
#include <iostream>
#include <tuple>
#include <vector>

binary_csr::binary_csr(
    const std::map<int, std::vector<int>> &unique_patterns,
    std::map<std::vector<int>, std::vector<int>> &shared_patterns,
    int no_rows) {
  row_ptr.reserve(no_rows + 1);
  row_ptr.push_back(0);

  // Process unique patterns (row-wise storage)
  for (int row = 0; row < no_rows; ++row) {
    auto it = unique_patterns.find(row);
    if (it != unique_patterns.end()) {
      const std::vector<int> &cols = it->second;
      col_indices.insert(col_indices.end(), cols.begin(), cols.end());
      data.insert(data.end(), cols.size(), 1.0f);
      row_ptr.push_back(row_ptr.back() + cols.size());
    } else {
      row_ptr.push_back(row_ptr.back());
    }
  }

  suffix_row_ptr.push_back(0);
  map_suffix_ptr.push_back(0);
  for (const auto &pair : shared_patterns) {
    const std::vector<int> &pattern_cols = pair.second;

    suffix_col_indices.insert(suffix_col_indices.end(), pattern_cols.begin(),
                              pattern_cols.end());
    suffix_data.insert(suffix_data.end(), pattern_cols.size(), 1.0f);
    suffix_row_ptr.push_back(suffix_row_ptr.back() + pattern_cols.size());
    std::vector rows_to_map = pair.first;
    map_row_index.insert(map_row_index.end(), rows_to_map.begin(),
                         rows_to_map.end());
    map_suffix_ptr.push_back(rows_to_map.size());
  }
}

void binary_csr::print() const {
  std::cout << "Row pointers: [";
  for (size_t i = 0; i < row_ptr.size(); ++i) {
    std::cout << row_ptr[i];
    if (i != row_ptr.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;

  std::cout << "Column indices: [";
  for (size_t i = 0; i < col_indices.size(); ++i) {
    std::cout << col_indices[i];
    if (i != col_indices.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;

  std::cout << "Data: [";
  for (size_t i = 0; i < data.size(); ++i) {
    std::cout << data[i];
    if (i != data.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;
}

void binary_csr::print_dense_matrix() const {
  int num_rows = row_ptr.size() - 1;
  int num_cols = 0;

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
const std::vector<int> &binary_csr::get_row_ptr() const { return row_ptr; }

const std::vector<int> &binary_csr::get_col_indices() const {
  return col_indices;
}

const std::vector<float> &binary_csr::get_data() const { return data; }

const std::tuple<std::vector<int>, std::vector<int>>
binary_csr::get_mapped_rows() const {
  return std::make_tuple(map_suffix_ptr, map_row_index);
}

const std::vector<int> &binary_csr::get_suffix_row_ptr() const {
  return suffix_row_ptr;
};

const std::vector<int> &binary_csr::get_suffix_col_indices() const {

  return suffix_col_indices;
};

const std::vector<float> &binary_csr::get_suffix_data() const {
  return suffix_data;
};
