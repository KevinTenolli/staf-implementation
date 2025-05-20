#ifndef SUFFIX_TRIE_HPP
#define SUFFIX_TRIE_HPP

#include "trie_node.hpp"
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class trie_node; // Forward declaration

class suffix_trie {
private:
  /**
   * @brief The root node of the suffix trie.
   * Uses unique_ptr for automatic memory management.
   */
  std::unique_ptr<trie_node> root;

  /**
   * @brief Helper function to perform a "true" insertion of a node into the
   * trie.
   *
   * @param node Pointer to the node to insert.
   * @param parent Optional pointer to the parent node; nullptr if root-level.
   */
  void true_insert_node(trie_node *node, trie_node *parent = nullptr);

  /**
   * @brief Recursively delete nodes marked as "false" insertions.
   *
   * @param node Pointer to the node to delete.
   */
  void delete_false_node(trie_node *node);

  /**
   * @brief Builds shared patterns bottom-up from the trie starting at a given
   * node.
   *
   * @param node Pointer to the node to start pattern extraction from.
   * @param patterns Map from row indeces vectors to shared column indeces
   * vectors
   * @return Set of rows of the previous node.
   */
  std::set<int> build_patterns_bottom_up(
      const trie_node *node,
      std::map<std::vector<int>, std::vector<int>> &patterns);

  /**
   * @brief Builds unique patterns bottom-up, associating  row indeces to unique
   * column indeces vectors
   *
   * @param node Pointer to the node to start pattern extraction from.
   * @param patterns Map from row indeces to vector of associated column indeces
   * vector.
   * @return Set of rows of the previous node.
   */
  std::set<int> build_patterns_bottom_up_unique(
      const trie_node *node, std::map<int, std::vector<int>> &patterns) const;

  /**
   * @brief Search for a node in the trie that contains a specific row index.
   *
   * @param node Starting node for the search.
   * @param row The row index to search for.
   * @return Pointer to the node containing the row, or nullptr if not found.
   */
  trie_node *search_node_with_row(trie_node *node, int32_t row);

  /**
   * @brief Recursively prints the trie nodes for visualization/debugging.
   *
   * @param node Pointer to the current node to print.
   * @param prefix String prefix for formatting the tree structure.
   * @param is_last Boolean indicating if the current node is the last sibling
   * (for formatting).
   */
  void print_node(const trie_node *node, const std::string &prefix,
                  bool is_last) const;

public:
  /**
   * @brief Constructs an empty suffix_trie.
   */
  suffix_trie();

  /**
   * @brief Extracts and returns shared patterns in the trie.
   *
   * @return A map where keys are a vector of rows and values are the shared
   * column indeces.
   */
  std::map<std::vector<int>, std::vector<int>> get_shared_patterns();

  /**
   * @brief Extracts and returns unique patterns from the trie.
   *
   * @return A map where keys are row numbers and values are the unique column
   * indeces for the rows
   */
  std::map<int, std::vector<int>> get_unique_patterns();

  /**
   * @brief Attempts to insert rows into the trie as "false" insertions.
   * Used during pattern building and insertion phases.
   *
   * @param col The current column index for insertion.
   * @param rows Pointer to the array of row indices to insert.
   * @param size Number of rows to insert.
   * @return Calculated score for the column insertion.
   */
  int false_insert(int col, const int32_t *rows, int size);

  /**
   * @brief Performs a "true" insertion phase in the trie after all false
   * insertions.
   */
  void true_insert();

  /**
   * @brief Deletes all nodes marked as "false" insertions to clean the trie.
   */
  void delete_false_nodes();

  /**
   * @brief Checks if the trie is empty.
   *
   * @return true if the trie has no nodes, false otherwise.
   */
  bool is_empty();

  /**
   * @brief Prints the entire trie structure to stdout for debugging.
   */
  void print_trie();
};

#endif
