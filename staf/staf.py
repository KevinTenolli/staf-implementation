import torch
from staf import staf_cpp as staf_cpp
import numpy as np
import json


class staf():

    def __init__(self, edge_index, edge_values, l, m, dataset):

        n_rows = n_cols = max(edge_index[0].max(), edge_index[1].max()) + 1

        csc_tensor = torch.sparse_coo_tensor(
            edge_index.to(torch.int32),
            edge_values.to(torch.float32),
            (n_rows, n_cols)
        ).coalesce().to_sparse_csc()

        result = staf_cpp.init_staf(
            csc_tensor.ccol_indices().to(dtype=torch.int32),
            csc_tensor.row_indices().to(dtype=torch.int32),
            csc_tensor.values().to(dtype=torch.float32),
            n_rows, n_cols, l, m
        )
        csr_tensors = result[0]
        suffix_tensors = result[1]
        map_tensors = result[2]
        torch.save(csr_tensors, f"csr_{dataset}_m_{m}_l_{l}.pt")
        torch.save(suffix_tensors, f"suffix_{dataset}_m_{m}_l_{l}.pt")
        torch.save(map_tensors, f"map_{dataset}_m_{m}_l_{l}.pt")
