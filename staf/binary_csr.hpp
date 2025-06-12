#ifndef BINARY_CSR_HPP
#define BINARY_CSR_HPP

#include <map>
#include <vector>

/**
 * @class binary_csr
 * @brief Represents a binary sparse matrix in Compressed Sparse Row (CSR)
 * format.
 *
 * This class takes in unique and shared patterns to construct a binary CSR
 * matrix where values are either 1.0 (present) or 0.0 (absent). It provides
 * functionality to print the CSR structure and also its dense representation.
 */
class binary_csr {
private:
  std::vector<int> row_ptr;
  std::vector<int> col_indices;
  std::vector<float> data;
  std::vector<int> suffix_row_ptr;
  std::vector<int> suffix_col_indices;
  std::vector<float> suffix_data;
  std::vector<int> map_suffix_ptr;
  std::vector<int> map_row_index;

public:
  /**
   * @brief Constructs the CSR matrix from unique and shared patterns.
   *
   * @param unique_patterns A map of row indices to vectors of column indices.
   *                        Represents the unique entries per row.
   * @param shared_patterns A map of common column patterns mapped to the list
   * of rows that share them. These are appended after unique patterns.
   * @param no_rows The total number of rows in the matrix.
   */
  binary_csr(const std::map<int, std::vector<int>> &unique_patterns,
             std::map<std::vector<int>, std::vector<int>> &shared_patterns,
             int no_rows);

  /**
   * @brief Prints the CSR structure (row_ptr, col_indices, and data).
   */
  void print() const;

  /**
   * @brief Prints the matrix in dense form to the console.
   */
  void print_dense_matrix() const;

  /**
   * @brief Returns the row pointer array of the CSR matrix.
   * @return const reference to the row_ptr vector.
   */
  const std::vector<int> &get_row_ptr() const;

  /**
   * @brief Returns the column indices of the CSR matrix.
   * @return const reference to the col_indices vector.
   */
  const std::vector<int> &get_col_indices() const;

  /**
   * @brief Returns the data values of the CSR matrix.
   * @return const reference to the data vector.
   */
  const std::vector<float> &get_data() const;

  /**
   * @brief Returns the row pointer array of the CSR matrix.
   * @return const reference to the row_ptr vector.
   */
  const std::vector<int> &get_suffix_row_ptr() const;

  /**
   * @brief Returns the column indices of the CSR matrix.
   * @return const reference to the col_indices vector.
   */
  const std::vector<int> &get_suffix_col_indices() const;

  /**
   * @brief Returns the data values of the CSR matrix.
   * @return const reference to the data vector.
   */
  const std::vector<float> &get_suffix_data() const;

  const std::tuple<std::vector<int>, std::vector<int>> get_mapped_rows() const;
};

#endif
