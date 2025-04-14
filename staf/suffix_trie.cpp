#include "trie_node.cpp"
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

  std::set<int> build_patterns_bottom_up(
      const trie_node *node,
      std::map<std::vector<int>, std::vector<int>> &patterns) const {
    std::set<int> current_rows = node->get_row_numbers();
    bool is_shared = node->is_shared();
    bool is_leaf = node->get_children().empty();

    // Process children first (post-order traversal)
    for (const auto &child : node->get_children()) {
      std::set<int> child_rows =
          build_patterns_bottom_up(child.get(), patterns);
      current_rows.insert(child_rows.begin(), child_rows.end());
    }

    if (is_shared || (is_leaf && current_rows.size() > 1)) {
      std::vector<int> current_pattern = {};
      if (node->get_index() >= 0) {
        current_pattern.push_back(node->get_index());
      }
      // Only store if actually shared by multiple rows
      std::vector<int> key(current_rows.begin(), current_rows.end());
      patterns[key] = current_pattern;
    } else if (current_rows.size() > 1) {
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

public:
  suffix_trie(size_t size)
      : root(std::make_unique<trie_node>()), vector_size(size) {}
  // utility function to print the nodes
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

  void print_trie() {
    if (!root) {
      std::cout << "Trie is empty" << std::endl;
      return;
    }
    std::cout << "Suffix Trie Structure:" << std::endl;
    print_node(root.get(), "", true);
  }
  void add_matrix_row(const std::vector<float> &row, int row_num) {
    auto *node = root.get();
    bool is_last_non_zero = false;
    for (int i = row.size() - 1; i >= 0; --i) {
      if (row[i] != 0) {
        node = node->has_child(i) ? node->get_child(i) : node->add_child(i);
        if (i == 0 || row[i - 1] == 0) {
          is_last_non_zero = true;
        }

        // If this is the last non-zero element, add the row number to the node
        if (is_last_non_zero) {
          node->add_row_number(row_num);
        }
      }
    }
  }

  std::map<std::vector<int>, std::vector<int>> get_shared_patterns() const {
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

  std::map<int, std::vector<int>> get_unique_patterns() const {
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
};
