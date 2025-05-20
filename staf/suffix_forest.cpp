// suffix_forest.cpp
#include "suffix_forest.hpp"
#include <iostream>
#include <limits>

suffix_forest::suffix_forest() {}

size_t suffix_forest::size() const {
  return tries.size();
}

suffix_trie* suffix_forest::get_trie(size_t index) {
  if (index < tries.size()) {
    return tries[index].get();
  }
  return nullptr;
}

void suffix_forest::create_forest(const int32_t* col_ptr, const int32_t* row_ind, int num_cols) {
  for (int col = num_cols - 1; col >= 0; --col) {
    int start = col_ptr[col];
    int end = col_ptr[col + 1];
    int count = end - start;

    const int32_t* rows = &row_ind[start];
    std::cout << "=========column" << col << std::endl;
    int selected_trie = false_insert_all(col, rows, count);
    true_insert(selected_trie);
  }
}

int suffix_forest::false_insert_all(int col, const int32_t* rows, int count) {
  std::cout << "Column " << col << ": Rows = [";
  for (int i = 0; i < count; ++i) {
    std::cout << rows[i];
    if (i < count - 1) std::cout << ", ";
  }
  std::cout << "]" << std::endl;

  if (tries.empty() || !tries.back()->is_empty()) {
    if (!tries.empty()) {
      tries.back()->print_trie();
    }
    tries.emplace_back(std::make_unique<suffix_trie>());
  }

  std::tuple<int, int> optimal_trie{-1, std::numeric_limits<int>::max()};
  for (size_t i = 0; i < tries.size(); ++i) {
    int score = tries[i]->false_insert(col, rows, count);
    std::cout << "trie " << i << " score: " << score << std::endl;
    if (score < std::get<1>(optimal_trie)) {
      optimal_trie = {i, score};
    }
  }
  return std::get<0>(optimal_trie);
}

void suffix_forest::true_insert(int selected_trie) {
  for (size_t i = 0; i < tries.size(); ++i) {
    if (selected_trie == i) {
      std::cout << "=======inserting trie" << i << std::endl;
      tries[i]->true_insert();
    } else {
      std::cout << "=========deleting trie" << i << std::endl;
      tries[i]->delete_false_nodes();
    }
  }
}

void suffix_forest::print_forest() {
  for (size_t i = 0; i < tries.size(); ++i) {
    std::cout << "Trie nr " << i << std::endl;
    tries[i]->print_trie();
    tries[i]->get_unique_patterns();
    tries[i]->get_shared_patterns();
  }
}
