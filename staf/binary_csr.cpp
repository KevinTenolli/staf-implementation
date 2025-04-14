#include <iostream>
#include <map>
#include <vector>
class binary_csr {
private:
  std::vector<int> col_indices;
  std::vector<int> row_ptr;

public:
  binary_csr(const std::map<int, std::vector<int>> &patterns) {
    for (const auto &[row, columns] : patterns) {
      row_ptr.push_back(row);
      col_indices.insert(col_indices.end(), columns.begin(), columns.end());
    }
  }
  binary_csr(const std::map<std::vector<int>, std::vector<int>> &patterns) {
    int index = 0;
    for (const auto &[_row, columns] : patterns) {
      row_ptr.push_back(index);
      col_indices.insert(col_indices.end(), columns.begin(), columns.end());
      index++;
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
  }
};
