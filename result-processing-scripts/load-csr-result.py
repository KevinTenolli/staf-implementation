import torch
import numpy as np
import os
import json


def main():
    # Get paths for input files relative to this script
    current_dir = os.path.dirname(__file__)
    parent_dir = os.path.abspath(os.path.join(current_dir, os.pardir))
    csr_file_name = os.path.join(parent_dir, "csr_tensors.pt")
    shared_patterns_file_name = os.path.join(
        parent_dir, "shared_patterns.json")

    # Check if files exist
    if not os.path.exists(csr_file_name):
        print(f"Error: CSR file '{csr_file_name}' not found.")
        return

    if not os.path.exists(shared_patterns_file_name):
        print(f"Error: Shared patterns file '{
              shared_patterns_file_name}' not found.")
        return

    # Load CSR data (PyTorch tensors -> numpy arrays for efficient processing)
    try:
        data = torch.load(csr_file_name)
        row_ptr = data['row_ptr'].cpu().numpy()
        col_indices = data['col_indices'].cpu().numpy()
        values = data['values'].cpu().numpy()
    except Exception as e:
        print(f"Failed to load CSR data: {e}")
        return

    # Load shared patterns JSON data (contains patterns and n_rows for splitting)
    try:
        with open(shared_patterns_file_name, "r") as f:
            shared_data = json.load(f)
            patterns = shared_data.get("patterns", [])
            n_rows = shared_data.get("n_rows")
    except Exception as e:
        print(f"Failed to load shared patterns: {e}")
        return

    # === Convert CSR to COO format using numpy for efficiency ===
    # Calculate number of columns by taking max of col_indices + 1 (if data exists)
    n_cols = int(col_indices.max()) + 1 if len(col_indices) > 0 else 0

    # Total number of nonzeros in the sparse matrix
    total_nnz = len(values)

    # row_counts = number of nonzeros per row
    row_counts = np.diff(row_ptr)

    # Construct COO row indices by repeating each row index according to number of nonzeros in that row
    rows = np.repeat(np.arange(len(row_ptr) - 1), row_counts)
    cols = col_indices
    vals = values

    # === Split COO entries into two parts: part1 (rows < n_rows), part2 (rows >= n_rows) ===
    part1_mask = rows < n_rows
    part2_mask = ~part1_mask

    # Extract part1 (rows within first n_rows)
    rows1, cols1, vals1 = rows[part1_mask], cols[part1_mask], vals[part1_mask]

    # Extract part2 (rows beyond n_rows)
    rows2, cols2, vals2 = rows[part2_mask], cols[part2_mask], vals[part2_mask]

    # Start final COO arrays with part1 data copied over
    final_rows = rows1.copy()
    final_cols = cols1.copy()
    final_vals = vals1.copy()

    # Number of rows in part2 block (used for iterating patterns)
    num_part2_rows = 0 if len(rows2) == 0 else rows2.max() - n_rows + 1

    # === Expand part2 rows according to shared patterns ===
    if num_part2_rows > 0:
        # For each pattern index i and corresponding part2 row (n_rows + i)
        for i in range(min(len(patterns), num_part2_rows)):
            abs_row = n_rows + i

            # Mask for entries corresponding to this part2 row
            mask = (rows2 == abs_row)
            target_cols = cols2[mask]
            target_vals = vals2[mask]

            # Target rows from pattern i to replicate data onto (filter to < n_rows)
            target_rows = np.array(patterns[i], dtype=np.int64)
            target_rows = target_rows[target_rows < n_rows]

            # Skip if no valid target rows or no columns/values to replicate
            if len(target_rows) == 0 or len(target_cols) == 0:
                continue

            # Efficient replication:
            # Repeat each target_row for every column in target_cols
            repeated_rows = np.repeat(target_rows, len(target_cols))

            # Tile columns and values to align with repeated rows
            tiled_cols = np.tile(target_cols, len(target_rows))
            tiled_vals = np.tile(target_vals, len(target_rows))

            # Append these replicated entries to final COO arrays
            final_rows = np.concatenate((final_rows, repeated_rows))
            final_cols = np.concatenate((final_cols, tiled_cols))
            final_vals = np.concatenate((final_vals, tiled_vals))

    # === Convert final COO arrays back to PyTorch tensors ===
    final_rows_t = torch.from_numpy(final_rows).long()
    final_cols_t = torch.from_numpy(final_cols).long()
    final_vals_t = torch.from_numpy(final_vals)

    # === Build final sparse COO tensor WITHOUT specifying size explicitly ===
    # Let PyTorch infer the size from the indices automatically
    final_coo = torch.sparse_coo_tensor(
        torch.stack([final_rows_t, final_cols_t]),
        final_vals_t
    ).coalesce()  # coalesce to sum duplicates and sort

    print("\n✅ Final Combined COO Tensor:")
    print(final_coo)

    # Convert to CSC format for column-based operations
    final_csc = final_coo.to_sparse_csc()
    print("\n✅ Final Combined CSC Tensor:")
    print(final_csc)

    # === Example expected tensors for validation (replace with your own data) ===
    expected_row_indices = torch.tensor(
        [0, 2, 3, 4, 0, 2, 3, 4, 1, 1, 4, 0, 2, 3, 4, 0, 2, 3, 4], dtype=torch.int32)
    expected_col_pointers = torch.tensor(
        [0, 4, 8, 9, 11, 15, 19], dtype=torch.int32)

    # Get actual CSC structure arrays for validation
    actual_row_indices = final_csc.row_indices()
    actual_col_pointers = final_csc.ccol_indices()

    print("\n📌 Expected Row Indices:", expected_row_indices)
    print("📌 Actual   Row Indices:", actual_row_indices)
    print("✅ Match:", torch.equal(expected_row_indices, actual_row_indices))

    print("\n📌 Expected Column Pointers:", expected_col_pointers)
    print("📌 Actual   Column Pointers:", actual_col_pointers)
    print("✅ Match:", torch.equal(expected_col_pointers, actual_col_pointers))


if __name__ == "__main__":
    main()
