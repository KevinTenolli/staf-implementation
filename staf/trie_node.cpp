#include <memory>
#include <set>
#include <vector>

class trie_node {
private:
  int index; // column number for the node
  std::vector<std::unique_ptr<trie_node>> children;
  std::set<int> row_numbers; // practically the leaves of the trie

public:
  trie_node() : index(-1) {}         // root node
  trie_node(int idx) : index(idx) {} // node

  trie_node *add_child(int idx) {
    for (auto &child : children) {
      if (child->index == idx)
        return child.get();
    }
    children.emplace_back(std::make_unique<trie_node>(idx));
    return children.back().get();
  }

  trie_node *get_child(int idx) const {
    for (auto &child : children) {
      if (child->index == idx)
        return child.get();
    }
    return nullptr;
  }

  bool has_child(int idx) const { return get_child(idx) != nullptr; }
  void add_row_number(int row_num) { row_numbers.insert(row_num); }
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
};
