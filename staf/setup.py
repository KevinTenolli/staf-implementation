# To build the extension, follow the instructions below:
# Set the MKLROOT environment variables to the path of your MKL installation.
# This can be done by running `source /path/to/setvars.sh` in your terminal.
# Run `python setup.py build_ext --inplace` in the `staf` directory.

from setuptools import setup, find_packages
from torch.utils.cpp_extension import BuildExtension, CppExtension
import os

extra_compile_args = [
    '-fopenmp',
    '-march=native',
    '-O2',
    '-std=c++20',
]

extra_link_args = []

install_requires = [
    "torch>=2.0.0",
]

setup(
    name='staf_cpp',
    packages=find_packages(),
    install_requires=install_requires,
    version="0.1",
    ext_modules=[
        CppExtension(
            'staf_cpp',
            [
                'staf_extensions.cpp',
                'suffix_forest.cpp',
                'suffix_trie.cpp',
                'binary_csr.cpp',
                'trie_node.cpp'
            ],
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args,
        ),
    ],
    cmdclass={
        'build_ext': BuildExtension
    }
)
