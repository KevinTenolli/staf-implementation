#include <iostream>
#include <map>
#include <omp.h>
#include <ostream>
#include <sstream>
#include <torch/extension.h>
#include <vector>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

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

class suffix_trie {
private:
  std::unique_ptr<trie_node> root;
  size_t vector_size;

  std::set<int> build_patterns_bottom_up(
      const trie_node *node,
      std::map<std::vector<int>, std::vector<float>> &patterns) const {
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
      std::vector<float> current_pattern = {};
      if (node->get_index() >= 0) {
        current_pattern[node->get_index()] = 1.0f;
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
          it->second[node->get_index()] = 1.0f; // Add 1.0 at current index
        }
      }
    }
    return current_rows;
  }

  std::set<int> build_patterns_bottom_up_unique(
      const trie_node *node,
      std::map<std::vector<int>, std::vector<float>> &patterns) const {
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
      return current_rows;
    }
    if (current_rows.size() == 1) {
      std::vector<int> key(current_rows.begin(), current_rows.end());
      auto it = patterns.find(key);
      if (it != patterns.end()) {
        // Pattern exists - modify it
        if (node->get_index() >= 0) {
          it->second[node->get_index()] = 1.0f; // Add 1.0 at current index
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

  std::map<std::vector<int>, std::vector<float>> get_shared_patterns() const {
    std::map<std::vector<int>, std::vector<float>> patterns;
    build_patterns_bottom_up(root.get(), patterns);
    std::cout << "ssssPatterns Map:\n";

    for (const auto &pair : patterns) {
      // Print the key (std::vector<int>)
      std::cout << "Key: [ ";
      for (int num : pair.first) {
        std::cout << num << " ";
      }
      std::cout << "] -> Values: [ ";

      // Print the value (std::vector<float>)
      for (float num : pair.second) {
        std::cout << num << " ";
      }
      std::cout << "]\n";
    }
    return patterns;
  }

  std::map<std::vector<int>, std::vector<float>> get_unique_patterns() const {
    std::map<std::vector<int>, std::vector<float>> patterns;
    build_patterns_bottom_up_unique(root.get(), patterns);
    std::cout << "unique Patterns Map:\n";

    for (const auto &pair : patterns) {
      // Print the key (std::vector<int>)
      std::cout << "Key: [ ";
      for (int num : pair.first) {
        std::cout << num << " ";
      }
      std::cout << "] -> Values: [ ";

      // Print the value (std::vector<float>)
      for (float num : pair.second) {
        std::cout << num << " ";
      }
      std::cout << "]\n";
    }
    return patterns;
  }
};

std::vector<std::vector<float>> create_matrix(int32_t *array_of_rows,
                                              int32_t *array_of_cols,
                                              float *array_of_values,
                                              int n_rows, int n_cols,
                                              float initial_value = 0.0f) {
  std::vector<std::vector<float>> matrix(
      n_rows, std::vector<float>(n_cols, initial_value));
  for (int i = 0; i < n_rows; i++) {
    int row = array_of_rows[i];
    int col = array_of_cols[i];
    matrix[row][col] = array_of_values[i];
  }
  return matrix;
}

std::vector<torch::Tensor> init_staf_(const torch::Tensor &row_idx,
                                      const torch::Tensor &col_idx,
                                      const torch::Tensor &values,
                                      const size_t n_rows,
                                      const size_t n_cols) {

  CHECK_DTYPE(row_idx, torch::kInt32);
  CHECK_DTYPE(col_idx, torch::kInt32);
  CHECK_DTYPE(values, torch::kFloat32);

  int32_t *array_of_rows = row_idx.data_ptr<int32_t>();
  int32_t *array_of_cols = col_idx.data_ptr<int32_t>();
  float *array_of_values = values.data_ptr<float>();

  auto base_matrix = create_matrix(array_of_rows, array_of_cols,
                                   array_of_values, n_rows, n_cols);

  /* auto trie = suffix_trie(3); */
  /**/
  /* // Add all rows to the trie (in reverse order for suffix trie) */
  /* trie.add_matrix_row({1, 0, 1}, 0); */
  /* trie.add_matrix_row({1, 1, 1}, 1); */
  /* trie.add_matrix_row({0, 1, 1}, 2); */
  /* auto patterns = trie.get_shared_patterns(); */
  /* trie.print_trie(); */
  // Get all patterns from the trie
  /* auto patterns = trie.get_shared_patterns(); */
  /**/
  // Create trie with known vector size (n_cols)
  auto trie2 = suffix_trie(3);
  trie2.add_matrix_row({1, 1, 1}, 1);
  trie2.add_matrix_row({1, 1, 1}, 2);
  trie2.add_matrix_row({0, 1, 1}, 3);

  // Get all patterns from the trie
  auto patterns2 = trie2.get_shared_patterns();
  trie2.get_unique_patterns();

  trie2.print_trie();

  // Convert patterns to torch tensors (placeholder - modify as needed)
  std::vector<torch::Tensor> result;
  // ... implementation to convert patterns to tensors ...

  return result;
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
