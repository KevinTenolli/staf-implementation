import argparse
from time import time
from torch import inference_mode, rand, empty, tensor
from utilities import load_dataset, print_dataset_info, set_adjacency_matrix

import warnings
warnings.simplefilter("ignore", UserWarning)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--operation", default='staf', choices=[
        "staf", "mkl-ax", "mkl-adx", "mkl-dadx"
    ], required=True)
    parser.add_argument("--dataset", type=str, default="ca-HepPh")
    parser.add_argument("--columns", type=int, default=500,
                        help="Overwrites default number of columns in matrix 'x'.")
    parser.add_argument("--iterations", type=int, default=50,
                        help="Overwrites default number of matrix multiplications tests.")
    parser.add_argument(
        "--l", type=int, default=2, help="Overwrites default lambda value for scoring")
    parser.add_argument(
        "--m", type=int, default=10, help="Overwrites default number of tries")
    parser.add_argument("--warmup", type=int, default=10,
                        help="Number of warmup iterations.")
    args = parser.parse_args()

    # Load dataset
    dataset = load_dataset(args.dataset)
    print_dataset_info(f"{args.dataset}", dataset)
    print('before build of format')
    # Convert adjacency matrices in the format specified in '--operation'
    a = set_adjacency_matrix(
        args.operation, dataset.edge_index, l=args.l, m=args.m)

    performance = []
    with inference_mode():
        x = rand((dataset.num_nodes, args.columns))
        y = empty((dataset.num_nodes, args.columns))
        for iterations in range(1, args.warmup + args.iterations + 1):
            time_start = time()

            # matrix multiplication
            a.matmul(x, y)

            time_end = time()
            performance.append(time_end - time_start)

    performance = tensor(performance[args.warmup:])

   # alpha_string = f" [alpha: {alpha}] " if "cbm-" in args.operation else " "
    # print(f"[{args.operation}] [{args.dataset}]{alpha_string}[columns: {args.columns}] [iterations: {args.iterations}] [warmups: {args.warmup}]   Mean: {
    #       performance.mean():.6f} s   |   Std: {performance.std():.6f} s   |   Min: {performance.min():.6f} s   |   Max: {performance.max():.6f} s")
