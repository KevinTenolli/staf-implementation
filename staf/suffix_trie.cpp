#include "trie_node.cpp"
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class suffix_trie {
private:
  std::unique_ptr<trie_node> root;
  size_t vector_size;

  /*
   * postorder traversal of a trie
   * Args:
   * Patterns (the map that holds the rows that share elements between them
   * [key]:[vector<int>], and the indeces that are shared [values]:vector<int>)
   * Returns: the rows that the previous row shares
   * Since a node does not know which rows are located at the end of the paths
   * between it, we propogate the rows up
   */
  std::set<int> build_patterns_bottom_up(
      const trie_node *node,
      std::map<std::vector<int>, std::vector<int>> &patterns) {
    std::set<int> current_rows = node->get_row_numbers();
    bool is_shared = node->is_shared();
    bool is_leaf = node->get_children().empty();

    // Process children first (post-order traversal)
    for (const auto &child : node->get_children()) {
      std::set<int> child_rows =
          build_patterns_bottom_up(child.get(), patterns);
      current_rows.insert(child_rows.begin(), child_rows.end());
    }
    /* if the node is shared or if its a leaf and it has more than 1 row at its
     end, from this point up to the other point whith the same characteristics
     or the end, the pattern is shared between many rows, we add this new
     pattern to the map
     */
    if (is_shared || (is_leaf && current_rows.size() > 1)) {
      std::vector<int> current_pattern = {};
      if (node->get_index() >= 0) {
        current_pattern.push_back(node->get_index());
      }
      // Only store if actually shared by multiple rows
      std::vector<int> key(current_rows.begin(), current_rows.end());
      if (!current_pattern.empty()) {
        patterns[key] = current_pattern;
      }
    } else if (current_rows.size() > 1) {
      /* if the list of rows that the node path owns matches an entry in the
       * map(matches the key), we add the node value(column index) to the map
       */

      std::vector<int> key(current_rows.begin(), current_rows.end());
      auto it = patterns.find(key);
      if (it != patterns.end()) {
        // Pattern exists - modify it
        if (node->get_index() >= 0) {
          it->second.push_back(node->get_index());
        }
      }
    }
    return current_rows;
  }

  std::set<int> build_patterns_bottom_up_unique(
      const trie_node *node, std::map<int, std::vector<int>> &patterns) const {
    std::set<int> current_rows = node->get_row_numbers();
    bool is_shared = node->is_shared();
    bool is_leaf = node->get_children().empty();

    // Process children first (post-order traversal)
    for (const auto &child : node->get_children()) {
      std::set<int> child_rows =
          build_patterns_bottom_up_unique(child.get(), patterns);
      current_rows.insert(child_rows.begin(), child_rows.end());
    }

    if (is_shared || (is_leaf && current_rows.size() > 1)) {
      return current_rows;
    }
    if (current_rows.size() == 1 && is_leaf) {
      std::vector<int> current_pattern = {};
      if (node->get_index() >= 0) {
        current_pattern.push_back(node->get_index());
      }
      int key = *current_rows.begin();
      patterns[key] = current_pattern;
    } else if (current_rows.size() == 1) {
      int key = *current_rows.begin();
      auto it = patterns.find(key);
      if (it != patterns.end()) {
        // Pattern exists - modify it
        if (node->get_index() >= 0) {
          it->second.push_back(node->get_index());
        }
      }
    }
    return current_rows;
  }

  void print_node(const trie_node *node, const std::string &prefix,
                  bool is_last) const {
    std::cout << prefix << (is_last ? "└── " : "├── ");
    std::cout << (node->get_index() == -1
                      ? "ROOT"
                      : "Index " + std::to_string(node->get_index()));

    if (!node->get_row_numbers().empty()) {
      std::cout << " (rows:";
      for (int row : node->get_row_numbers())
        std::cout << " " << row;
      std::cout << ")";
    }
    std::cout << std::endl;

    const auto &children = node->get_children();
    for (size_t i = 0; i < children.size(); ++i) {
      print_node(children[i].get(), prefix + (is_last ? "    " : "│   "),
                 i == children.size() - 1);
    }
  }

public:
  /*
   * Constructor
   * Initializes the trie
   * Input size:t_size the number of columns in a matrix, which is the longest
   * length of a trie path
   */
  suffix_trie(size_t size)
      : root(std::make_unique<trie_node>()), vector_size(size) {}

  void build_trie_from_tensors(int32_t *rows, int32_t *columns,
                               int num_elements) {
    auto *node = root.get();
    auto previous_row = -1; // Initialize to an invalid row

    // Process elements in reverse order
    for (int i = num_elements - 1; i >= 0; i--) {
      int current_row = rows[i];
      int current_col = columns[i];

      // If we've moved to a new row, reset to root
      if (previous_row != current_row) {
        node = root.get();
        previous_row = current_row;
      }

      // Add the column as a child
      node = node->has_child(current_col) ? node->get_child(current_col)
                                          : node->add_child(current_col);

      // If this is the first element in reverse order for this row, store the
      // row number (which means it's the last non-zero element when considering
      // forward order)
      if (i == 0 || rows[i - 1] != current_row) {
        node->add_row_number(current_row);
      }
    }
  }
  /*
   * method to get a map for the shared patterns(rows and the indeces they
   * share)
   * returns -> patterns: map, a map which has:
   * row numbers as key: vector<int>
   * indeces where the rows have a non 0 as value: vector<int>
   */
  std::map<std::vector<int>, std::vector<int>> get_shared_patterns() {
    std::map<std::vector<int>, std::vector<int>> patterns;
    build_patterns_bottom_up(root.get(), patterns);
    std::cout << "Shared patterns map:\n";

    for (const auto &pair : patterns) {
      std::cout << "Rows: [ ";
      for (int num : pair.first) {
        std::cout << num << " ";
      }
      std::cout << "] -> Columns: [ ";

      for (int num : pair.second) {
        std::cout << num << " ";
      }
      std::cout << "]\n";
    }
    return patterns;
  }

  /*
   * method to get a map for the unique patterns(rows that dont have shared
   * elements)
   * returns -> patterns: map, a map which has:
   * row number as key: int
   * indeces where the row is non 0 as value: vector<int>
   */
  std::map<int, std::vector<int>> get_unique_patterns() {
    std::map<int, std::vector<int>> patterns;
    build_patterns_bottom_up_unique(root.get(), patterns);
    std::cout << "Unique patterns map:\n";

    for (const auto &pair : patterns) {
      std::cout << "Row: [ ";
      std::cout << pair.first << " ";
      std::cout << "] -> Columns: [ ";
      for (int num : pair.second) {
        std::cout << num << " ";
      }
      std::cout << "]\n";
    }
    return patterns;
  }

  /*
   * ONLY USE FOR TESTING AND DEBUGGING SMALL MATRICES
   * method to print the trie
   */
  void print_trie() {
    if (!root) {
      std::cout << "Trie is empty" << std::endl;
      return;
    }
    std::cout << "Suffix Trie Structure:" << std::endl;
    print_node(root.get(), "", true);
  }
};
