# STAF: Sparse Trie-based Adjacency Forest

This project implements a fast sparse pattern detection algorithm using a suffix forest data structure, written in C++ with Python bindings through PyTorch's C++ extensions.

👉 GitHub Repository: [KevinTenolli/staf-implementation](https://github.com/KevinTenolli/staf-implementation)

---

## 🚀 How to Run

Follow these steps to set up your environment and run the benchmark.

---

### ✅ Setup & Execution

```bash
# 1. Clone the repository
git clone https://github.com/KevinTenolli/staf-implementation.git
cd staf-implementation

# 2. Create and activate a virtual environment
python3 -m venv venv
source venv/bin/activate

# 3. Install required Python packages
pip install torch torch-geometric

# Optional: install all dependencies from a requirements file
# pip install -r requirements.txt

# 4. Set the PYTHONPATH so Python can find the local packages
export PYTHONPATH=$PYTHONPATH:.

# 5. Build the C++ extension in-place
cd staf
python setup.py build_ext --inplace
cd ..

# 6. Run the benchmark
python3 benchmark/example.py --format staf

