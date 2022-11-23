import torch
import numpy as np

def matrix_to_txt(name, matrix):
    with open(f"{name}-{'-'.join(str(dim) for dim in matrix.shape)}.txt", "w") as f:
        lines = []
        for r in matrix:
            line = " ".join(f"{num:.8f}" for num in r) + "\n"
            lines.append(line)
        f.writelines(lines)


if __name__ == "__main__":
    in_dim = (512, 512)
    dim1 = (512, 512)
    
    in_m = torch.rand(*in_dim).detach().numpy()

    linear = torch.nn.Linear(*dim1)
    _ = torch.nn.init.xavier_uniform_(linear.weight)
    weight = linear.weight.detach().numpy()

    out_m = np.matmul(in_m, weight)
    
    matrix_to_txt("in-rand", in_m)
    matrix_to_txt("w-xav", weight)
    matrix_to_txt("in-rand-w-xav", out_m)
