#ifndef SUFFIX_FOREST_HPP
#define SUFFIX_FOREST_HPP

#include "suffix_trie.hpp"
#include <cstdint>
#include <memory>
#include <vector>

class suffix_forest {
public:
  /**
   * @brief Constructs an empty suffix_forest.
   */
  suffix_forest();

  /**
   * @brief Returns the number of suffix tries in the forest.
   * @return The number of tries stored in the forest.
   */
  size_t size() const;

  /**
   * @brief Access a suffix_trie pointer at the specified index.
   * @param index The index of the trie to retrieve.
   * @return Pointer to the suffix_trie at the given index.
   */
  suffix_trie *get_trie(size_t index);

  /**
   * @brief Build the forest of suffix tries from a CSC sparse matrix's column
   * pointer and row indices.
   *
   * @param col_ptr Pointer to the array of column start indices (size num_cols
   * + 1).
   * @param row_ind Pointer to the array of row indices corresponding to
   * non-zero entries.
   * @param num_cols Number of columns in the matrix.
   */
  void create_forest(const int32_t *col_ptr, const int32_t *row_ind,
                     int num_cols);

  /**
   * @brief Print a representation of the entire suffix forest to stdout.
   * Useful for debugging and visualization.
   */
  void print_forest();

private:
  /**
   * @brief Container holding the suffix tries in the forest.
   * Each suffix_trie corresponds to a structure built from matrix columns.
   */
  std::vector<std::unique_ptr<suffix_trie>> tries;

  /**
   * @brief Attempts to insert a block of rows into the forest as "false"
   * inserts. Used internally for forest construction.
   *
   * @param col The current column index being processed.
   * @param rows Pointer to the array of row indices to insert.
   * @param count Number of rows to insert.
   * @return An integer status or identifier related to insertion
   * success/failure.
   */
  int false_insert_all(int col, const int32_t *rows, int count);

  /**
   * @brief Performs a "true" insertion in the selected trie after certain
   * checks. For all other tries, delete the temporary inserted nodes.
   * @param selected_trie Index of the trie to insert into.
   */
  void true_insert(int selected_trie);
};

#endif
