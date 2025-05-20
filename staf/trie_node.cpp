#include <memory>
#include <set>
#include <vector>

class trie_node {
private:
  int index; // column number for the node
  std::vector<std::unique_ptr<trie_node>> children;
  std::set<int> row_numbers; // practically the leaves of the trie
  bool false_insert;

public:
  trie_node() {
    index = -1;
    false_insert = false;
  } // root node
  trie_node(int idx, bool false_insert = false) {
    index = idx;
    this->false_insert = false_insert;
  } // node

  trie_node *add_child(int idx, bool fi = false) {
    for (auto &child : children) {
      if (child->index == idx)
        return child.get();
    }
    children.emplace_back(std::make_unique<trie_node>(idx, fi));
    return children.back().get();
  }

  trie_node *get_child(int idx) const {
    for (auto &child : children) {
      if (child->index == idx)
        return child.get();
    }
    return nullptr;
  }
  void remove_row(int row) { row_numbers.erase(row); }

  bool has_child(int idx) const { return get_child(idx) != nullptr; }
  void add_row_number(int row_num) { row_numbers.insert(row_num); }
  bool has_row_number(int row_num) { return row_numbers.count(row_num) > 0; }
  void clear_row_numbers() { row_numbers.clear(); }
  const std::set<int> &get_row_numbers() const { return row_numbers; }
  const std::vector<std::unique_ptr<trie_node>> &get_children() const {
    return children;
  }
  int get_index() const { return index; }
  bool is_shared() const {
    if (children.size() >= 2 || row_numbers.size() >= 2 ||
        (children.size() >= 1 && row_numbers.size() >= 1)) {
      return true;
    }
    return false;
  }
  bool is_empty() {
    if (children.size() == 0 && row_numbers.size() == 0) {
      return true;
    }
    return false;
  }
  bool is_false_inserted() { return false_insert; }

  void true_insert() { false_insert = false; }
  void remove_child_if_false_inserted() {
    for (auto it = children.begin(); it != children.end();) {
      if ((*it)->is_false_inserted()) {
        it = children.erase(it);
      } else {
        ++it;
      }
    }
  }
};
