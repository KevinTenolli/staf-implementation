#include "suffix_forest.hpp"
#include <iostream>
#include <limits>

suffix_forest::suffix_forest(size_t nr_tries, size_t score_lambda) {
  this->nr_tries = nr_tries;
  this->score_lambda = score_lambda;
}

size_t suffix_forest::size() const { return tries.size(); }

suffix_trie *suffix_forest::get_trie(size_t index) {
  if (index < tries.size()) {
    return tries[index].get();
  }
  return nullptr;
}

void suffix_forest::create_forest(const int32_t *col_ptr,
                                  const int32_t *row_ind, int num_cols) {
  for (int col = num_cols - 1; col >= 0; col--) {
    int start = col_ptr[col];
    int end = col_ptr[col + 1];
    int count = end - start;
    std::cout << "iteration " << col << "/" << num_cols - 1 << std::endl;

    const int32_t *rows = &row_ind[start];
    int selected_trie = false_insert_all(col, rows, count);
    true_insert(selected_trie);
  }
}

int suffix_forest::false_insert_all(int col, const int32_t *rows, int count) {
  if (tries.size() < this->nr_tries &&
      (tries.empty() || !tries.back()->is_empty())) {
    tries.emplace_back(std::make_unique<suffix_trie>());
  }

  std::tuple<int, int> global_optimal{-1, std::numeric_limits<int>::max()};

#pragma omp parallel
  {
    std::tuple<int, int> local_optimal{-1, std::numeric_limits<int>::max()};

#pragma omp for nowait
    for (int i = 0; i < static_cast<int>(tries.size()); ++i) {
      int score = tries[i]->false_insert(col, rows, count, this->score_lambda);
      if (score < std::get<1>(local_optimal)) {
        local_optimal = {i, score};
      }
    }

#pragma omp critical
    {
      if (std::get<1>(local_optimal) < std::get<1>(global_optimal)) {
        global_optimal = local_optimal;
      }
    }
  }

  return std::get<0>(global_optimal);
}

void suffix_forest::true_insert(int selected_trie) {
#pragma omp parallel for
  for (size_t i = 0; i < tries.size(); i++) {
    if (selected_trie == i) {
      tries[i]->true_insert();
    } else {
      tries[i]->delete_false_nodes();
    }
  }
}

binary_csr suffix_forest::build_csr(int n_rows) {
  std::map<int, std::vector<int>> combined_unique_patterns;
  std::map<std::vector<int>, std::vector<int>> combined_shared_patterns;

  int max_row = 0;

  for (const auto &trie : tries) {
    auto up = trie->get_unique_patterns();
    auto sp = trie->get_shared_patterns();

    for (const auto &[row, cols] : up) {
      combined_unique_patterns[row].insert(combined_unique_patterns[row].end(),
                                           cols.begin(), cols.end());
      if (row > max_row)
        max_row = row;
    }

    for (const auto &[key, val] : sp) {
      combined_shared_patterns[key].insert(combined_shared_patterns[key].end(),
                                           val.begin(), val.end());
    }
  }
  binary_csr csr(combined_unique_patterns, combined_shared_patterns, n_rows);
  return csr;
}

void suffix_forest::print_forest() {
  for (size_t i = 0; i < tries.size(); i++) {
    std::cout << "Trie nr " << i << std::endl;
    tries[i]->print_trie();
    tries[i]->get_unique_patterns();
    tries[i]->get_shared_patterns();
  }
}
