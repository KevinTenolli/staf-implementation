import torch
import numpy as np
import os
import json
from numba import njit
import time


def load_data_files(parent_dir):
    """Load all required data files with error handling"""
    csr_file = os.path.join(parent_dir, "csr_tensors.pt")
    patterns_file = os.path.join(parent_dir, "shared_patterns.json")
    ref_file = os.path.join(parent_dir, "data.pt")

    if not all(os.path.exists(f) for f in [csr_file, patterns_file, ref_file]):
        missing = [f for f in [csr_file, patterns_file,
                               ref_file] if not os.path.exists(f)]
        raise FileNotFoundError(f"Missing files: {missing}")

    try:
        csr_data = torch.load(csr_file)
        with open(patterns_file, 'r') as f:
            patterns_data = json.load(f)
        ref_data = torch.load(ref_file)
        return csr_data, patterns_data, ref_data['csc']
    except Exception as e:
        raise RuntimeError(f"Data loading failed: {str(e)}")


@njit(parallel=True)
def expand_patterns_numba(row_ptr, col_indices, values, patterns, n_rows, num_part2_rows):
    """Numba-optimized pattern expansion"""
    max_possible_size = len(col_indices) * (1 + max(len(p) for p in patterns))
    expanded_rows = np.empty(max_possible_size, dtype=np.int64)
    expanded_cols = np.empty(max_possible_size, dtype=np.int64)
    expanded_vals = np.empty(max_possible_size, dtype=values.dtype)

    current_idx = 0

    for row in range(n_rows):
        start = row_ptr[row]
        end = row_ptr[row + 1]
        for i in range(start, end):
            expanded_rows[current_idx] = row
            expanded_cols[current_idx] = col_indices[i]
            expanded_vals[current_idx] = values[i]
            current_idx += 1

    for i in range(min(len(patterns), num_part2_rows)):
        abs_row = n_rows + i
        start = row_ptr[abs_row]
        end = row_ptr[abs_row + 1]

        if start == end:
            continue

        target_cols = col_indices[start:end]
        target_vals = values[start:end]
        target_rows = np.array(
            [r for r in patterns[i] if r < n_rows], dtype=np.int64)

        if len(target_rows) == 0:
            continue

        for row in target_rows:
            for col, val in zip(target_cols, target_vals):
                expanded_rows[current_idx] = row
                expanded_cols[current_idx] = col
                expanded_vals[current_idx] = val
                current_idx += 1

    return expanded_rows[:current_idx], expanded_cols[:current_idx], expanded_vals[:current_idx]


def reconstruct_matrix(csr_data, patterns_data):
    """Efficient reconstruction of original matrix from compressed form"""
    row_ptr = csr_data['row_ptr'].cpu().numpy()
    col_indices = csr_data['col_indices'].cpu().numpy()
    values = csr_data['values'].cpu().numpy()
    patterns = patterns_data['patterns']
    n_rows = patterns_data['n_rows']

    total_rows = len(row_ptr) - 1
    num_part2_rows = max(0, total_rows - n_rows)

    t_start = time.time()
    rows, cols, vals = expand_patterns_numba(
        row_ptr, col_indices, values, patterns, n_rows, num_part2_rows
    )
    t_elapsed = time.time() - t_start
    print(f"Pattern expansion completed in {t_elapsed:.2f} seconds")

    indices = torch.stack([
        torch.from_numpy(rows).long(),
        torch.from_numpy(cols).long()
    ])
    values_t = torch.from_numpy(vals)

    return torch.sparse_coo_tensor(indices, values_t).coalesce()


def main():
    try:
        parent_dir = os.path.abspath(os.path.join(
            os.path.dirname(__file__), os.pardir))
        csr_data, patterns_data, ref_csc = load_data_files(parent_dir)

        reconstructed = reconstruct_matrix(csr_data, patterns_data)
        reconstructed_csc = reconstructed.to_sparse_csc()

        print("\n=== Verification ===")
        rows_match = torch.equal(
            reconstructed_csc.row_indices(), ref_csc.row_indices())
        cols_match = torch.equal(
            reconstructed_csc.ccol_indices(), ref_csc.ccol_indices())

        if rows_match and cols_match:
            print("✅ Perfect reconstruction achieved!")
        else:
            print("❌ Reconstruction differs from reference")

        original_size = sum(t.numel() for t in [
                            ref_csc.ccol_indices(), ref_csc.row_indices(), ref_csc.values()])
        compressed_size = sum(t.numel() for t in [
                              csr_data['row_ptr'], csr_data['col_indices'], csr_data['values']])
        patterns_size = sum(len(p) for p in patterns_data['patterns'])

        print(f"\n=== Compression Metrics ===")
        print(f"Original size: {original_size:,} elements")
        print(f"Compressed size: {compressed_size:,} elements")
        print(f"Patterns size: {patterns_size:,} elements")
        print(f"Compression ratio: {original_size /
              (compressed_size + patterns_size):.2f}x")

    except Exception as e:
        print(f"Error: {str(e)}")
        return 1

    return 0


if __name__ == "__main__":
    main()
