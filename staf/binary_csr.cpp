#include <iostream>
#include <map>
#include <vector>
class binary_csr {
private:
  std::vector<int> row_ptr;
  std::vector<int> col_indices;
  std::vector<float> data;

public:
  binary_csr(const std::map<int, std::vector<int>> &unique_patterns,
             std::map<std::vector<int>, std::vector<int>> &shared_patterns,
             int no_rows) {
    row_ptr.reserve(no_rows + 1);
    row_ptr.push_back(0);

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

    for (const auto &pair : shared_patterns) {
      const std::vector<int> &pattern_cols = pair.second;

      col_indices.insert(col_indices.end(), pattern_cols.begin(),
                         pattern_cols.end());
      data.insert(data.end(), pattern_cols.size(), 1.0f);
      row_ptr.push_back(row_ptr.back() + pattern_cols.size());
    }
  }
  void print() const {
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

  const std::vector<int> &get_row_ptr() const { return row_ptr; }
  const std::vector<int> &get_col_indices() const { return col_indices; }
  const std::vector<float> &get_data() const { return data; }
};
