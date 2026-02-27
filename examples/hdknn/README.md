# powersqueeze examples

This directory includes examples illustrating the use of `hdknn` on top of
`powersqueeze` to construct and query a knn index using power iteration
representations.
See the code in, e.g., `power_iteration_dnnd_kron.cpp` for inline descriptions
of the types and operations.

## power_iteration_dnnd_kron.cpp

This file illustrates `hdknn`'s basic functionality on Kronecker graphs - graphs
whose topology is the result of a Kronecker product of two smaller graph
matrices.
This implementation performs Kronecker products of tsv data that is
organized like the streaming partition challenge datasets from the HPEC
[graph challenge dataset](https://graphchallenge.mit.edu/data-sets).
In addition to the `adjacency_fn` and `sketch_fn` functors that occurred in the
`power_iteration_tsv.cpp` source, this code also includes functors that wrap the
index construction and querying procedure of the `dnnd` algorithm implemented in
`saltatlas`, as well as several metrics that consume the resulting
neighborhoods.

The `adjacency_fn` functor in this example consumes two input files and computes
a Kronecker product adjacency matrix, implemented in `psqz::kron::adjacency`.
In order to avoid making this matrix too dense, it stochastically thins the
matrix by sampling intra-community product edges and inter-community product
edges with user-specified probabilities that are given in the CLI.
It is possible to make these probabilities too small, which will result in
empty rows/columns of the adjacency matrix and will disrupt any downstream
metrics.
This code checks and notifies the user if there are any empty rows of the
adjacency matrix (or the sketch representations).

### CLI

You can view the CLI of the `power_iteration_dnnd_kron` executable by running

```
$ ./examples/hdknn/power_iteration_dnnd_kron -h
Usage:
 -h             Indicates whether to print usage string
 -c <arg>       Number of vertices in the input graph
 -R <arg>       Random seed
 -B <arg>       Buffer size used during exponentiation (default \infty)
 -M <arg>       Metall datastore path (optional)
 -u             Indicates that the graph stream is undirected
 -m             Indicates ygm::map use (ygm::array otherwise)
 -V             Indicates whether to print verbose output
 -i <arg>       File containing data for left kronecker graph (required)
 -I <arg>       File containing data for right kronecker graph (required)
 -g <arg>       File containing community data for left kronecker graph (required)
 -G <arg>       File containing community data for right kronecker graph (required)
 -a <arg>       power (a>1.0) of expected mean degree log(|V(A)| * |V(B)|)^a
 -b <arg>       ratio (b>1.0) of intra-community to inter-community preservation
 -r <arg>       Power of 2 size of projection range
 -e <arg>       Integral power of data matrix
 -C             Perform chebyshev polynomial embedding
 -k <arg>       Number of nearest neighbors in index
 -K <arg>       Number of nearest neighbors to query
 -P <arg>       DNND: rho parameter
 -D <arg>       DNND: delta parameter
 -S <arg>       DNND: batch size parameter
 -U             DNND: make index undirected?
 -L <arg>       DNND: pruning degree multiplier
 -E <arg>       DNND: epsilon parameter
```

Most of these arguments are explained in the description of
`power_iteration_kron.cpp`.
Those explanations are not repeated here.

Note that there is no `-q` flag, as it is not obvious how or why one would
query a subset of the Kronecker graph.

`-k` indicates the number of nearest neighbors to use in index
construction.
`-K` indicates the number of nearest neighbors to query.
`-P`, `-D`, `-S`, `-U`, `-L`, and `-E` are all DNND-specific parameters that
are provided to the CLI for convenience.

### Example invocation

An example invocation on LC, using the prepared saltaltas benchmarks, might look
like

```
srun -N 1 --tasks-per-node 36 -p pbatch -A seq \
  ./examples/hdknn/power_iteration_dnnd_kron \
  -i /p/lustre1/salta/benchmarks/graphchallenge/2017/100/simulated_blockmodel_graph_100_nodes.tsv \
  -I /p/lustre1/salta/benchmarks/graphchallenge/2017/100/simulated_blockmodel_graph_100_nodes.tsv \
  -g /p/lustre1/salta/benchmarks/graphchallenge/2017/100/simulated_blockmodel_graph_100_nodes_truePartition.tsv \
  -G /p/lustre1/salta/benchmarks/graphchallenge/2017/100/simulated_blockmodel_graph_100_nodes_truePartition.tsv \
  -c 10000 -k 10 -K 5 -r 8 -e 3 -V -a 1.4 -b 4.0
```

This can be made a bit simpler using some convenience scripts within the repo,
namely

```
srun -N 1 --tasks-per-node 36 -p pbatch -A seq \
  $(../scripts/gc_kron.sh ./examples/hdknn/power_iteration_dnnd_kron 100 100) \
  -k 10 -K 5 -r 8 -e 3 -V -a 1.4 -b 4.0
```

This script simply auto-fills the `-i`, `-I`, `-g`, `-G`, and `-c` fields based
upon the hard-coded paths of the saltatlas benchmarks.
You need only specify two valid numbers of vertices
(e.g. 5000, 50000, 200000, etc) indicating the left and right graph challenge
graphs to use.
It is possible to compute products of a graph with itself, such as in the above
example.

## power_iteration_dnnd_kron_verbose.cpp

This code is semantically identical to `power_iteration_dnnd_kron.cpp`, but
writes out the interaction between the distributed sketch containers and `dnnd`
in long form.
Examining the source will hopefully make it more obvious how to use the
`sketch_container` objects produced by `psqz`.

## jaccard_dnnd_tsv.cpp

This code performs a drastically different workflow than
`power_iteration_dnnd_kron.cpp`. Instead of using powersqueeze to embed the
vertices, it instead approximates Jaccard distance $d_J(\mathbf{x}, \mathbf{y})$
for sparse boolean vectors $\mathbf{x}$ and $\mathbf{y}$ with

$
\begin{equation}
\hat{d}_J(\mathbf{x}, \mathbf{y})
= 1 - \frac{\left < S\mathbf{x}, S\mathbf{y} \right >}
{\|\mathbf{x}\| + \|\mathbf{y}\| - \left < S\mathbf{x}, S\mathbf{y} \right >},
\end{equation}
$
where $S$ is a conforming Johnson-Lindenstrauss transform.
Note that this embedding is likely to perform much worse than powersqueeze on
the graph challenge graphs, although it may have uses on other graphs or
term-document matrices.

**NOTE** - these examples use psqz parsers, so they expect square matrices.

### CLI

The interface to `jaccard_dnnd_tsv.cpp` is very similar to prior tsv examples
with the addition of dnnd parameters.
You can view the CLI by running
```
$ ./examples/hdknn/jaccard_dnnd_tsv -h
Usage:
 -h             Indicates whether to print usage string
 -c <arg>       Number of vertices in the input graph
 -z <arg>       Random seed
 -B <arg>       Buffer size used during exponentiation (default \infty)
 -M <arg>       Metall datastore path (optional)
 -u             Indicates that the graph stream is undirected
 -m             Indicates ygm::map use (ygm::array otherwise)
 -V             Indicates whether to print verbose output
 -i <arg>       File containing data to build index (required)
 -q <arg>       Optional file containing indices to query
 -g <arg>       File containing true communities
 -r <arg>       Power of 2 size of projection range
 -R <arg>       Power of 2 number of replicated projections
 -e <arg>       Integral power of data matrix
 -C             Perform chebyshev polynomial embedding
 -k <arg>       Number of nearest neighbors in index
 -K <arg>       Number of nearest neighbors to query
construction/querying.
 -P <arg>       DNND: rho parameter
 -D <arg>       DNND: delta parameter
 -S <arg>       DNND: batch size parameter
 -U             DNND: make index undirected?
 -L <arg>       DNND: pruning degree multiplier
 -E <arg>       DNND: epsilon parameter
```
The usage of these parameters is much the same as with other examples, except
that `-e` should not be given a value other than 1 (the default value).
If `-e` is set to `>1`, a power iteration will occur, and Equation (1) will
instead use higher order embeddings for the inner product, which is not
well-formed.

### Example usage

An example invocation on LC, using the prepared graphchallenge benchmarks,
might look like

```
srun -N 1 --tasks-per-node 36 -p pbatch -A seq \
./examples/hdknn/jaccard_dnnd_tsv \
-i /Users/priest2/workspace/nisenemarks/data/2017/5000/parts.txt \
-g /Users/priest2/workspace/nisenemarks/data/2017/5000/gt_parts.txt \
-c 5000 -k 10 -K 5 -r 32 -R 1 -e 1 -V
```

This can be made a bit simpler using some convenience scripts within the repo,
namely

```
srun -N 1 --tasks-per-node 36 -p pbatch -A seq \
  $(../scripts/gc_tsv.sh ./examples/hdknn/jaccard_dnnd_tsv 5000) \
  -k 10 -K 5 -r 32 -R 1 -e 1 -V
```

or (for better load-balancing during IO)

```
srun -N 1 --tasks-per-node 36 -p pbatch -A seq \
  $(../scripts/gc_parts.sh ./examples/hdknn/jaccard_dnnd_tsv 5000) \
  -k 10 -K 5 -r 32 -R 1 -e 1 -V
```

These scripts auto-fill the `-i`, `-g`, and `-c` fields based upon hard-coded
paths of the graph challenge benchmarks.
If you have a separate query file (i.e., you intend to query a subset of
vertices in the graph), then you will need to pass a file containing the
newline-separated list of queries (or a newline-separated list of paths to such
files, same as the other inputs) to the `-q` field.


## jaccard_dnnd_tsv_query_dump.cpp

This example is very similar to `jaccard_dnnd_tsv.cpp`, save that it does not
accept a ground truth argument and thus has no `-g` flag.
Instead of computing metrics, it gathers and dumps all the neighborhoods to a
file specified by the `-d` flag, which must be given.
Since there is no ground truth file, the user must provide a query file using
the `-q` flag.

The data dumped to file has the output format specified in `saltatlas::utility::gather_and_dump_neighbors`.

### CLI

The interface to `jaccard_dnnd_tsv_query_dump.cpp` is very similar to
`jaccard_dnnd_tsv.cpp` with the exceptions noted above.

```
$ ./examples/hdknn/jaccard_dnnd_tsv_query_dump -h
Usage:
 -h             Indicates whether to print usage string
 -c <arg>       Number of vertices in the input graph
 -z <arg>       Random seed
 -B <arg>       Buffer size used during exponentiation (default \infty)
 -M <arg>       Metall datastore path (optional)
 -u             Indicates that the graph stream is undirected
 -m             Indicates ygm::map use (ygm::array otherwise)
 -V             Indicates whether to print verbose output
 -i <arg>       File containing data to build index (required)
 -q <arg>       Optional file containing indices to query
 -r <arg>       Power of 2 size of projection range
 -R <arg>       Power of 2 number of replicated projections
 -e <arg>       Integral power of data matrix
 -C             Perform chebyshev polynomial embedding
 -k <arg>       Number of nearest neighbors in index
 -K <arg>       Number of nearest neighbors to query
 -P <arg>       DNND: rho parameter
 -D <arg>       DNND: delta parameter
 -S <arg>       DNND: batch size parameter
 -U             DNND: make index undirected?
 -L <arg>       DNND: pruning degree multiplier
 -E <arg>       DNND: epsilon parameter
 -d <arg>       Path to dump output files.
 ```

### Example Usage

An example invocation on LC, using the prepared graphchallenge benchmarks,
might look like

```
srun -N 1 --tasks-per-node 36 -p pbatch -A seq \
./examples/hdknn/jaccard_dnnd_tsv_query_dump \
-i /Users/priest2/workspace/nisenemarks/data/2017/5000/parts.txt \
-q /Users/priest2/workspace/nisenemarks/data/2017/5000/gt_parts.txt \
-c 5000 -k 10 -K -r 32 -R 1 -e 1 -V -d /path/to/outdir
```

One could also use a different query file as desired.
