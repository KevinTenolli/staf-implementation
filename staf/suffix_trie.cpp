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

  void true_insert_node(trie_node *node, trie_node *parent = nullptr) {
    for (auto &child : node->get_children()) {
      true_insert_node(child.get(), node);
    }

    if (node->is_false_inserted()) {
      node->true_insert();

      if (parent) {
        for (int row : node->get_row_numbers()) {
          parent->remove_row(row);
        }
      }
    }
  }

  void delete_false_node(trie_node *node) {
    if (!node)
      return;
    for (const auto &child : node->get_children()) {
      delete_false_node(child.get());
    }
    node->remove_child_if_false_inserted();
  }
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

  trie_node *search_node_with_row(trie_node *node, int32_t row) {
    if (node->has_row_number(row)) {
      return node;
    }
    for (const auto &child : node->get_children()) {
      trie_node *result = search_node_with_row(child.get(), row);
      if (result)
        return result;
    }
    return nullptr;
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
   */
  suffix_trie() : root(std::make_unique<trie_node>()) {}

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

  int false_insert(int col, const int32_t *rows, int size) {
    int new_nodes = 0;
    int new_rows = 0;

    for (int i = 0; i < size; i++) {
      int32_t row = rows[i];
      trie_node *node = search_node_with_row(root.get(), row);

      // Case 1: Row not found in trie — use root
      if (!node) {
        node = root.get();
        if (node->has_child(col)) {
          trie_node *child = node->get_child(col);
          child->add_row_number(row);
          new_rows++;
        } else {
          trie_node *inserted_node = node->add_child(col, true);
          inserted_node->add_row_number(row);
          new_nodes++;
          new_rows++;
        }
      }
      // Case 2: Row found in trie
      else {
        if (node->has_child(col)) {
          trie_node *child = node->get_child(col);
          child->add_row_number(row);
        } else {
          trie_node *inserted_node = node->add_child(col, true);
          inserted_node->add_row_number(row);
          new_nodes++;
        }
      }
    }

    return new_nodes * 2 + new_rows;
  }

  void true_insert() { true_insert_node(root.get()); }

  void delete_false_nodes() { delete_false_node(root.get()); }

  bool is_empty() { return root->is_empty(); }

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
