import torch
from staf import staf_cpp as staf_cpp
import numpy as np
import json


class staf():

    def __init__(self, edge_index, edge_values, l, m):

        # check if its right
        n_rows = n_cols = max(edge_index[0].max(), edge_index[1].max()) + 1

        csc_tensor = torch.sparse_coo_tensor(
            edge_index.to(torch.int32),
            edge_values.to(torch.float32),
            (n_rows, n_cols)
        ).coalesce().to_sparse_csc()
        torch.save({'csc': csc_tensor.cpu()}, "data.pt")

        result = staf_cpp.init_staf(
            csc_tensor.ccol_indices().to(dtype=torch.int32),
            csc_tensor.row_indices().to(dtype=torch.int32),
            csc_tensor.values().to(dtype=torch.float32),
            n_rows, n_cols, l, m
        )
        print(result[1])
        list_of_csr_tensors = result[0]

        torch.save({
            'row_ptr': list_of_csr_tensors[0],
            'col_indices': list_of_csr_tensors[1],
            'values': list_of_csr_tensors[2]
        }, "csr_tensors.pt")
        shared_patterns_data = result[1]
        try:
            data_to_save = {
                'n_rows': n_rows.item(),
                'patterns': shared_patterns_data
            }
            with open("shared_patterns.json", "w") as f:
                json.dump(data_to_save, f, indent=4)
            print("Shared patterns saved to shared_patterns.json")
        except Exception as e:
            print(f"Error saving shared patterns: {e}")
        print("Tensors saved to csr_tensors.pt")
