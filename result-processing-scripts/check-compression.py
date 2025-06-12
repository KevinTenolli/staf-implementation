import torch
import json
import os
import numpy as np
from pathlib import Path


def calculate_compression_ratio():
    parent_dir = Path(__file__).parent.parent

    original_file = parent_dir / "data.pt"
    compressed_file = parent_dir / "csr_tensors.pt"
    patterns_file = parent_dir / "shared_patterns.json"

    original_data = torch.load(original_file)
    csr_tensor = original_data['csc'].to_sparse_csr()

    original_size = (csr_tensor.crow_indices().numel() +
                     csr_tensor.col_indices().numel() +
                     csr_tensor.values().numel())
    original_values = csr_tensor.values().numel()
    print(f'number of values csr = {original_values}')

    compressed_data = torch.load(compressed_file)

    compressed_size = (compressed_data['row_ptr'].numel() +
                       compressed_data['col_indices'].numel() +
                       compressed_data['values'].numel())

    compressed_values = compressed_data['values'].numel()
    print(f'number of values csr = {compressed_values}')

    with open(patterns_file, "r") as f:
        shared_patterns_data = json.load(f)

    patterns = shared_patterns_data['patterns']
    shared_patterns_size = sum(
        np.array(p, dtype=object).size for p in patterns)

    total_compressed_size = compressed_size + shared_patterns_size

    compression_ratio = original_size / total_compressed_size

    print("=== Compression Ratio Calculation ===")
    print(f"Original size (total elements in CSR tensors): {original_size}")
    print(f"Compressed tensors size: {compressed_size}")
    print(f"Shared patterns size: {shared_patterns_size}")
    print(f"Total compressed size: {total_compressed_size}")
    print(
        f"\nCompression ratio (original/compressed): {compression_ratio:.2f}x")
    print(f"Space savings: {(1 - (1/compression_ratio))*100:.1f}%")

    print(f"Value ratio: {original_values/compressed_values}")
    return compression_ratio


if __name__ == "__main__":
    # Check if all required files exist
    parent_dir = Path(__file__).parent.parent
    required_files = [
        parent_dir / "data.pt",
        parent_dir / "csr_tensors.pt",
        parent_dir / "shared_patterns.json"
    ]

    missing_files = [f.name for f in required_files if not f.exists()]

    if missing_files:
        print(f"Error: Missing required files in parent directory: {
              missing_files}")
        print(f"Parent directory: {parent_dir}")
    else:
        calculate_compression_ratio()
