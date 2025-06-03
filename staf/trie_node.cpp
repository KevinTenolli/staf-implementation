#include "trie_node.hpp"

// Constructor for root node
trie_node::trie_node() : index(-1), false_insert(false) {}

// Constructor for node with index
trie_node::trie_node(int idx, bool false_insert)
    : index(idx), false_insert(false_insert) {}

trie_node* trie_node::add_child(int idx, bool fi) {
  for (auto& child : children) {
    if (child->get_index() == idx)
      return child.get();
  }
  children.emplace_back(std::make_unique<trie_node>(idx, fi));
  return children.back().get();
}

trie_node* trie_node::get_child(int idx) const {
  for (const auto& child : children) {
    if (child->get_index() == idx)
      return child.get();
  }
  return nullptr;
}

bool trie_node::has_child(int idx) const {
  return get_child(idx) != nullptr;
}

void trie_node::remove_child_if_false_inserted() {
  for (auto it = children.begin(); it != children.end();) {
    if ((*it)->is_false_inserted()) {
      it = children.erase(it);
    } else {
      ++it;
    }
  }
}

void trie_node::add_row_number(int row_num) {
  row_numbers.insert(row_num);
}

void trie_node::remove_row(int row) {
  row_numbers.erase(row);
}

bool trie_node::has_row_number(int row_num) {
  return row_numbers.count(row_num) > 0;
}

void trie_node::clear_row_numbers() {
  row_numbers.clear();
}

const std::set<int>& trie_node::get_row_numbers() const {
  return row_numbers;
}

int trie_node::get_index() const {
  return index;
}

const std::vector<std::unique_ptr<trie_node>>& trie_node::get_children() const {
  return children;
}

bool trie_node::is_shared() const {
  return (children.size() >= 2 ||
          row_numbers.size() >= 2 ||
          (children.size() >= 1 && row_numbers.size() >= 1));
}

bool trie_node::is_empty() {
  return children.empty() && row_numbers.empty();
}

bool trie_node::is_false_inserted() {
  return false_insert;
}

void trie_node::true_insert() {
  false_insert = false;
}
