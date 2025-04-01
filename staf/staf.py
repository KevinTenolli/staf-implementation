import torch
from staf import staf_cpp as staf_cpp

class staf():

    def __init__(self, edge_index, edge_values):

        n_rows = n_cols = max(edge_index[0].max(), edge_index[1].max()) + 1

        # convert matrix of deltas to COO tensor (torch.float32)
        coo_tensor = torch.sparse_coo_tensor(
            edge_index.to(torch.int32), 
            edge_values.to(torch.float32), 
            (n_rows, n_cols)
        ).coalesce()

        staf_cpp.init_staf(coo_tensor.indices()[0].to(dtype=torch.int32),
                           coo_tensor.indices()[1].to(dtype=torch.int32),
                           coo_tensor.values().to(dtype=torch.float32),
                           n_rows, n_cols)
