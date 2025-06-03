from staf.staf import staf
from torch import zeros, ones, int32, float32, arange, randint

# dataset packages
from ogb.nodeproppred import PygNodePropPredDataset
from torch_geometric.data import Batch
from torch_geometric.datasets import TUDataset, SuiteSparseMatrixCollection, Planetoid

############################################################
########################## LAYERS ##########################
############################################################


def set_adjacency_matrix(format, edge_index, l, m):
    if format == "staf":
        return staf(edge_index.to(int32), ones(edge_index.size(1), dtype=float32), l, m)
    else:
        raise NotImplementedError(f"Format {format} is not valid")

############################################################
######################### DATASETS #########################
############################################################

# examples:
# load_tudataset('PROTEINS', 'node')
# load_tudataset('PROTEINS', 'graph')
# load_tudataset('COLLAB', 'node')
# load_tudataset('COLLAB', 'graph')
# ...


def print_dataset_info(name, dataset):
    print('------------------------------------------------------')
    print(f'Dataset: {name}')
    print(f'Number of Nodes: {dataset.num_nodes}')
    print(f'Number of Edges: {dataset.num_edges}')
    print(f'X: {dataset.x.shape if dataset.x is not None else None}')
    print('------------------------------------------------------')

# the following dataset loaders create a suitable feature matrix with OneHotDeg.
# for our purpososes these matrices are not needed, and are replaced with random
# feature and weighted matrices in benchmark_inference.py.


def load_tudataset(name):     # graph and node prediction
    dataset = Batch.from_data_list(TUDataset(root="../data", name=name))
    return dataset


def load_snap(name):      # node prediction
    dataset = SuiteSparseMatrixCollection(
        root="../data", name=name, group='SNAP')[0]
    return dataset

# examples:
# load_planetoid('PubMed')


def load_planetoid(name):       # node prediction
    dataset = Planetoid(root="../data", name=name)[0]
    return dataset

# examples:
# load_dimacs('coPapersCiteseer')


def load_dimacs(name):      # node prediction
    dataset = SuiteSparseMatrixCollection(
        root="../data", name=name, group='DIMACS10')[0]
    return dataset

# The second parameter returned represents CBM's alpha value. In this script all
# all alpha values were set to 16 which was shown to work well in a multi-core
# setting. To find the best alpha values for each dataset (and experimental
# environment) run alpha_searcher.sh and add them to load_dataset.


def load_dataset(name):
    if name == "ca-HepPh":
        return load_snap("ca-HepPh")
    elif name == "ca-AstroPh":
        return load_snap("ca-AstroPh")
    elif name == "PubMed":
        return load_planetoid("PubMed")
    elif name == "Cora":
        return load_planetoid("Cora")
    elif name == "coPapersCiteseer":
        return load_dimacs("coPapersCiteseer")
    elif name == "coPapersDBLP":
        return load_dimacs("coPapersDBLP")
    elif name == "COLLAB":
        return load_tudataset("COLLAB")
    else:
        raise NotImplementedError(f"Dataset {name} is not valid")
