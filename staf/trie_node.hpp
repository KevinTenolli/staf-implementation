#ifndef TRIE_NODE_HPP
#define TRIE_NODE_HPP

#include <memory>
#include <set>
#include <vector>

class trie_node {
public:
  /**
   * @brief Constructs a root trie node with no index.
   */
  trie_node();

  /**
   * @brief Constructs a trie node with a given index.
   *
   * @param idx The column index represented by this node.
   * @param false_insert Flag indicating if this node was inserted as a "false"
   * insertion.
   */
  trie_node(int idx, bool false_insert = false);

  /**
   * @brief Adds a child node with a given index.
   *
   * @param idx The index for the new child node.
   * @param fi Whether the child node is a false insertion.
   * @return Pointer to the newly created child node.
   */
  trie_node *add_child(int idx, bool fi = false);

  /**
   * @brief Gets a pointer to the child node with a specific index.
   *
   * @param idx The index of the child node to retrieve.
   * @return Pointer to the child node, or nullptr if not found.
   */
  trie_node *get_child(int idx) const;

  /**
   * @brief Checks if a child node with the given index exists.
   *
   * @param idx The index to check.
   * @return true if a child with the index exists, false otherwise.
   */
  bool has_child(int idx) const;

  /**
   * @brief Removes children that were inserted as "false" insertions.
   */
  void remove_child_if_false_inserted();

  /**
   * @brief Adds a row number to the current node.
   *
   * @param row_num The row number to add.
   */
  void add_row_number(int row_num);

  /**
   * @brief Removes a row number from the current node.
   *
   * @param row The row number to remove.
   */
  void remove_row(int row);

  /**
   * @brief Checks if the node contains a given row number.
   *
   * @param row_num The row number to check.
   * @return true if the row number exists in the node, false otherwise.
   */
  bool has_row_number(int row_num);

  /**
   * @brief Clears all stored row numbers from the node.
   */
  void clear_row_numbers();

  /**
   * @brief Gets the set of all row numbers associated with this node.
   *
   * @return Const reference to the set of row numbers.
   */
  const std::set<int> &get_row_numbers() const;

  /**
   * @brief Gets the index (column number) of this node.
   *
   * @return The node's index.
   */
  int get_index() const;

  /**
   * @brief Returns a const reference to the vector of child nodes.
   *
   * @return Vector of unique_ptrs to child trie_nodes.
   */
  const std::vector<std::unique_ptr<trie_node>> &get_children() const;

  /**
   * @brief Checks if the node is shared among multiple patterns.
   *
   * @return true if shared, false otherwise.
   */
  bool is_shared() const;

  /**
   * @brief Checks if the node has no children and no row numbers.
   *
   * @return true if the node is empty, false otherwise.
   */
  bool is_empty();

  /**
   * @brief Checks if the node was inserted as a false insertion.
   *
   * @return true if false inserted, false otherwise.
   */
  bool is_false_inserted();

  /**
   * @brief Marks the node as a "true" insertion, finalizing its state.
   */
  void true_insert();

private:
  int index; ///< Column number this node represents
  std::vector<std::unique_ptr<trie_node>> children; ///< Children nodes
  std::set<int> row_numbers;                        ///< Row indices (leaf data)
  bool false_insert; ///< Flag indicating if the node was a false insertion
};

#endif
