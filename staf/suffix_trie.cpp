#include "suffix_trie.hpp"
#include <iostream>

suffix_trie::suffix_trie() : root(std::make_unique<trie_node>()) {}

void suffix_trie::true_insert_node(trie_node *node, trie_node *parent) {
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

void suffix_trie::delete_false_node(trie_node *node) {
  if (!node)
    return;
  for (const auto &child : node->get_children()) {
    delete_false_node(child.get());
  }
  node->remove_child_if_false_inserted();
}

std::set<int> suffix_trie::build_patterns_bottom_up(
    const trie_node *node,
    std::map<std::vector<int>, std::vector<int>> &patterns) {
  std::set<int> current_rows = node->get_row_numbers();
  bool is_shared = node->is_shared();
  bool is_leaf = node->get_children().empty();

  for (const auto &child : node->get_children()) {
    std::set<int> child_rows = build_patterns_bottom_up(child.get(), patterns);
    current_rows.insert(child_rows.begin(), child_rows.end());
  }

  if (is_shared || (is_leaf && current_rows.size() > 1)) {
    std::vector<int> current_pattern;
    if (node->get_index() >= 0) {
      current_pattern.push_back(node->get_index());
    }
    std::vector<int> key(current_rows.begin(), current_rows.end());
    if (!current_pattern.empty()) {
      patterns[key] = current_pattern;
    }
  } else if (current_rows.size() > 1) {
    std::vector<int> key(current_rows.begin(), current_rows.end());
    auto it = patterns.find(key);
    if (it != patterns.end() && node->get_index() >= 0) {
      it->second.push_back(node->get_index());
    }
  }
  return current_rows;
}

std::set<int> suffix_trie::build_patterns_bottom_up_unique(
    const trie_node *node, std::map<int, std::vector<int>> &patterns) const {
  std::set<int> current_rows = node->get_row_numbers();
  bool is_shared = node->is_shared();
  bool is_leaf = node->get_children().empty();

  for (const auto &child : node->get_children()) {
    std::set<int> child_rows =
        build_patterns_bottom_up_unique(child.get(), patterns);
    current_rows.insert(child_rows.begin(), child_rows.end());
  }

  if (is_shared || (is_leaf && current_rows.size() > 1)) {
    return current_rows;
  }
  if (current_rows.size() == 1 && is_leaf) {
    std::vector<int> current_pattern;
    if (node->get_index() >= 0) {
      current_pattern.push_back(node->get_index());
    }
    int key = *current_rows.begin();
    patterns[key] = current_pattern;
  } else if (current_rows.size() == 1) {
    int key = *current_rows.begin();
    auto it = patterns.find(key);
    if (it != patterns.end() && node->get_index() >= 0) {
      it->second.push_back(node->get_index());
    }
  }
  return current_rows;
}

trie_node *suffix_trie::search_node_with_row(trie_node *node, int32_t row) {
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

void suffix_trie::print_node(const trie_node *node, const std::string &prefix,
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
  for (size_t i = 0; i < children.size(); i++) {
    print_node(children[i].get(), prefix + (is_last ? "    " : "│   "),
               i == children.size() - 1);
  }
}

std::map<std::vector<int>, std::vector<int>>
suffix_trie::get_shared_patterns() {
  std::map<std::vector<int>, std::vector<int>> patterns;
  build_patterns_bottom_up(root.get(), patterns);
  std::cout << "Shared patterns map:\n";

  for (const auto &pair : patterns) {
    std::cout << "Rows: [ ";
    for (int num : pair.first)
      std::cout << num << " ";
    std::cout << "] -> Columns: [ ";
    for (int num : pair.second)
      std::cout << num << " ";
    std::cout << "]\n";
  }
  return patterns;
}

std::map<int, std::vector<int>> suffix_trie::get_unique_patterns() {
  std::map<int, std::vector<int>> patterns;
  build_patterns_bottom_up_unique(root.get(), patterns);
  std::cout << "Unique patterns map:\n";

  for (const auto &pair : patterns) {
    std::cout << "Row: [ " << pair.first << " ] -> Columns: [ ";
    for (int num : pair.second)
      std::cout << num << " ";
    std::cout << "]\n";
  }
  return patterns;
}

int suffix_trie::false_insert(int col, const int32_t *rows, int size) {
  int new_nodes = 0;
  int new_rows = 0;

  for (int i = 0; i < size; i++) {
    int32_t row = rows[i];
    trie_node *node = search_node_with_row(root.get(), row);

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
    } else {
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

void suffix_trie::true_insert() { true_insert_node(root.get()); }

void suffix_trie::delete_false_nodes() { delete_false_node(root.get()); }

bool suffix_trie::is_empty() { return root->is_empty(); }

void suffix_trie::print_trie() {
  if (!root) {
    std::cout << "Trie is empty" << std::endl;
    return;
  }
  std::cout << "Suffix Trie Structure:" << std::endl;
  print_node(root.get(), "", true);
}
