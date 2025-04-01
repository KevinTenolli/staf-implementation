#include <iostream>
#include <omp.h>
#include <ostream>
#include <torch/extension.h>
#include <vector>
#include <map>
#include <sstream>

#define CHECK_DTYPE(x, dtype)                                                  \
  TORCH_CHECK(x.scalar_type() == dtype,                                        \
              "\"" #x "\" is not a tensor of type \"" #dtype "\"")

class trie_node {
private:
  int index; // column number for the node
  std::vector<std::unique_ptr<trie_node>> children;
  std::set<int> row_numbers; // practically the leaves of the trie

public:
  trie_node() : index(-1) {} // root node
  trie_node(int idx) : index(idx) {} // node

  // if node child exists, return it, else add it
  trie_node *add_child(int idx) {
    for (auto &child : children) {
      if (child->index == idx)
        return child.get();
    }
    children.emplace_back(std::make_unique<trie_node>(idx));
    return children.back().get();
  }

  //get child with the specific index if it exists
  trie_node *get_child(int idx) const {
    for (auto &child : children) {
      if (child->index == idx)
        return child.get();
    }
    return nullptr;
  }

  //check if child exists
  bool has_child(int idx) const { return get_child(idx) != nullptr; }
  //add a row number to a node
  void add_row_number(int row_num) { row_numbers.insert(row_num); }
  //get row numbers for a node
  const std::set<int> &get_row_numbers() const { return row_numbers; }
  //get children
  const std::vector<std::unique_ptr<trie_node>> &get_children() const {
    return children;
  }
  //return the column index for the node
  int get_index() const { return index; }

  // Check if node is shared by multiple rows
  bool is_shared() const { return row_numbers.size() > 1; }
};

class suffix_trie {
private:
  std::unique_ptr<trie_node> root;

  //utility function to print the nodes
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
  suffix_trie() : root(std::make_unique<trie_node>()) {}

  void add_matrix_row(const std::vector<float> &row, int row_num) {
    auto *node = root.get();
    for (int i = row.size() - 1; i >= 0; --i) {
      if (row[i] != 0) {
        node = node->has_child(i) ? node->get_child(i) : node->add_child(i);
        node->add_row_number(row_num); // Add row number to each node in the path
      }
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

    void traverse_paths() {
        auto *node = root.get();
    }
};

std::vector<std::vector<float>> create_matrix(int32_t *array_of_rows,
                                              int32_t *array_of_cols,
                                              float *array_of_values,
                                              int n_rows, int n_cols,
                                              float initial_value = 0.0f) {
  // Initialize a rows x cols matrix filled with initial_value
  std::vector<std::vector<float>> matrix(
      n_rows, std::vector<float>(n_cols, initial_value));
  // fill the matrix
  for (int i = 0; i < n_rows; i++) {
    int row = array_of_rows[i];
    int col = array_of_cols[i];
    matrix[row][col] = array_of_values[i];
  }
  return matrix;
}

// init_staf_ receives dataset in COO format
std::vector<torch::Tensor>
init_staf_(const torch::Tensor &row_idx, // COO row idx
           const torch::Tensor &col_idx, // COO col idx
           const torch::Tensor &values,  // COO values
           const size_t n_rows, const size_t n_cols) {

  CHECK_DTYPE(row_idx, torch::kInt32);
  CHECK_DTYPE(col_idx, torch::kInt32);
  CHECK_DTYPE(values, torch::kFloat32);

  int32_t *array_of_rows = row_idx.data_ptr<int32_t>();
  int32_t *array_of_cols = col_idx.data_ptr<int32_t>();
  float *array_of_values = values.data_ptr<float>();

  auto base_matrix = create_matrix(array_of_rows, array_of_cols,
                                   array_of_values, n_rows, n_cols);

  auto trie = suffix_trie();
  trie.add_matrix_row({1, 1, 1}, 0);
  trie.add_matrix_row({0, 1, 1}, 1);
  trie.add_matrix_row({1, 1, 0}, 2);
  trie.print_trie();
  
  
  std::cout << "inside init_staf_()\n";
  std::vector<torch::Tensor> result;

  return result;
}

// Actual python wrapper:
// 1st argument function name on python side,
// 2nd argument function name on cpp side.
PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) { m.def("init_staf", &init_staf_); }
