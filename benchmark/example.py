import argparse
from utilities import load_dataset, print_dataset_info, set_adjacency_matrix

import warnings
warnings.simplefilter("ignore", UserWarning)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--format", choices=["staf"], required=True)
    parser.add_argument("--dataset", type=str, default="ca-HepPh")
    args = parser.parse_args()

    # dataset
    dataset = load_dataset(args.dataset)
    print_dataset_info(f"{args.dataset}", dataset)

    # adjacency matrix
    a = set_adjacency_matrix(args.format, dataset.edge_index)
    del dataset.edge_index
