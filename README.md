# powersqueeze (psqz)

`powersqueeze` (abbreviated `psqz`) is an HPC-tuned truncated approximate power
iteration library that efficiently produces low-dimensional measurements of
graph matrix spectra by combining power iteration with sparse Johnson
Lindenstrauss transforms.

`psqz` depends on [YGM](https://github.com/LLNL/ygm) to handle distributed
containers and communication and [krowkee](https://github.com/LLNL/krowkee) to
implement Johnson Lindenstrauss transforms.

## Why use psqz?

`psqz` is an efficient tool to produce high-quality, fast, data-oblivious
low-dimensional representations of high-dimensional sparse data, such as graphs
or term-document matrices.
`psqz` randomly projects such a matrix into a low-dimensional subspace using
truncated power iteration, which converges (with some corrections) to the
truncated eigendecomposition of a square matrix.
Moreover, the operations used are efficient in distributed memory compared to
competing spectral measurements, such as parallel eigensolvers.

Say we have a matrix $A \in \mathbb{R}^{n \times n}$, where $n$ is so large
(and A is sufficiently dense) that $A$ cannot be stored on a single computer.
Computing an eigendecomposition of $A$ is hard and expensive, so `psqz` instead
samples a Johnson-Lindenstrauss matrix $S \in \mathbb{R}^{n \times r}$, where
$r \ll n$, and computes

$$
\begin{equation*}
A^tS = \underbrace{A (A \dots A(A(A}_t S)) \dots),
\end{equation*}
$$

computing the products from left to right.
This ensures that instead of computing square matrix products, we instead
perform $t$ sparse matrix multi-vector products, which are much more tractable.

The rows of $A^tS$ are then usable as $r$-dimensional representations of the
vertices of $A$, and their pairwise distances have been shown to correspond to
topological similarity in the graph.
Hence, rows of $A^tS$ with relatively low $\ell_2$ distance are likely to be
structurally similar to one another.
For example, a low $\ell_2$ distance between two such vertex representation
suggests that they are likely to participate in the same relatively dense
subregions or clusters in $A$.
Ergo, the rows of $A^tS$ can be used for downstream metric algorithms in the
same way that the truncated eigenvectors of the corresponding Laplacian are used
in spectral clustering.

Alternately, `psqz` also implements a fully dynamic streaming estimator by
sampling $t$ independent Johnson-Lindenstrauss matrices
$S_1, \dots, S_t \in \mathbb{R}^{n \times r}$ and approximates the power
iteration by interleaving them with products of $A$:

$$
\begin{equation*}
\widetilde{A}^{(t)} \approx A S_1 \sum_{i = 1}^{t - 1} S^{\top}_i A S_{i + 1}.
\end{equation*}
$$

The $\widetilde{A}^{(t)}$ estimator has several favorable properties compared
to $A^tS$, notably supporting arbitrary updates to $A$ and not requiring
distributed sparse matrix-multivector multiplications.
However, the effective dimension required by $\widetilde{A}^{(t)}$ to achieve
a low-distortion embedding is much higher than $A^tS$, limiting its utility in
some applications.

## hdknn

The `psqz` repository includes the `hdknn` library, which includes tools to
apply `psqz` representations to approximate k nearest neighbors querying using
the [saltatlas](https://github.com/LLNL/saltatlas) library.
`hdknn` is an optional compile target specified during the `cmake` build with
`-DPSQZ_INSTALL_HDKNN=ON`.
This option is `OFF` by default.

## Installation

`psqz` builds using `cmake` and a relatively modern gcc compiler.
We suggest gcc 11 or later, as earlier versions are no longer supported.

### Dependencies

- C++20
- GCC >= 11
- [krowkee](https://github.com/LLNL/krowkee)
  - Eigen3
- [YGM](https://github.com/LLNL/ygm)
  - [Cereal](https://github.com/USCiLab/cereal)
  - MPI
  - BOOST

If installing with `cmake`, all software dependencies will be automatically
fetched during the build.

### Optional dependencies

These additional dependencies are installed if also installing `hdknn`.

- [saltatlas](https://github.com/LLNL/saltatlas)

### How to build with cmake

Building this library with `cmake` is as simple as

```
$ git clone git@github.com:LLNL/powersqueeze.git
$ cd powersqueeze
$ mkdir build
$ cd build/
$ cmake .. -DCMAKE_BUILD_TYPE=RELEASE -DPSQZ_INSTALL_HDKNN=ON
```

Only set `PSQZ_INSTALL_HDKNN=ON` if you want to install the `saltatlas`
dependency and build the `hdknn` examples.
This `cmake` variable is set to `OFF` by default.

## Examples

See the `examples` directory and the READMEs therein for code examples using
`psqz` and `hdknn`.

## License

This project is licensed under the BSD-Commercial license - see the
[LICENSE](LICENSE) file for details.

## Release

LLNL-CODE-2014069