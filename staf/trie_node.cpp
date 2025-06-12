#include "trie_node.hpp"

trie_node::trie_node()
    : index(-1), row_numbers(), children(), parent(nullptr),
      false_insert(false) {}

trie_node::trie_node(int idx, bool fi)
    : index(idx), row_numbers(), children(), parent(nullptr), false_insert(fi) {
}

trie_node *trie_node::add_child(int idx, bool fi) {
  for (auto &child : children) {
    if (child->index == idx)
      return child.get();
  }
  auto new_child = std::make_unique<trie_node>(idx, fi);
  new_child->parent = this;
  children.push_back(std::move(new_child));
  return children.back().get();
}

trie_node *trie_node::get_child(int idx) const {
  for (size_t i = 0; i < children.size(); ++i) {
    if (children[i]->index == idx)
      return children[i].get();
  }
  return nullptr;
}

bool trie_node::has_child(int idx) const { return get_child(idx) != nullptr; }

void trie_node::remove_child_if_false_inserted() {
  auto it = children.begin();
  while (it != children.end()) {
    if ((*it)->false_insert) {
      it = children.erase(it);
    } else {
      ++it;
    }
  }
}

void trie_node::add_row_number(int row_num) { row_numbers.insert(row_num); }

void trie_node::remove_row(int row) { row_numbers.erase(row); }

bool trie_node::has_row_number(int row_num) {
  return row_numbers.find(row_num) != row_numbers.end();
}

void trie_node::clear_row_numbers() { row_numbers.clear(); }

const std::set<int> &trie_node::get_row_numbers() const { return row_numbers; }

int trie_node::get_index() const { return index; }

const std::vector<std::unique_ptr<trie_node>> &trie_node::get_children() const {
  return children;
}

bool trie_node::is_shared() const {
  return (children.size() >= 2 || row_numbers.size() >= 2 ||
          (children.size() >= 1 && row_numbers.size() >= 1));
}

bool trie_node::is_empty() { return children.empty() && row_numbers.empty(); }

bool trie_node::is_false_inserted() { return false_insert; }

void trie_node::true_insert() { false_insert = false; }

trie_node *trie_node::get_parent() const { return parent; }
