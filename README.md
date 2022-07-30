# frontier_basic_tdzdd

An example implementation of the frontier-based search
using TdZdd (https://github.com/kunisura/TdZdd ).

This program constructs a ZDD representing all the single cycles and a ZDD representing all the s-t paths on a given graph.

## Usage

```
make
./program --cycle --show grid3x3.txt
```

You will get the following:

```
Reading "grid3x3.txt" ... done in 0.00s elapsed, 0.00s user, 4MB.
# of vertices = 9
# of edges = 12
FrontierExampleSpec .......... <53> in 0.00s elapsed, 0.00s user, 4MB.
# of ZDD nodes = 53
# of solutions = 13
```

This constructs a ZDD representing all the single cycles.
If we specify an argument without '--', it is interpreted as
the input graph filename, which is in an edge list format.
The first edge (the first line in the file) corresponds to the variable (label) of the root of the constructed ZDD.
See the document in [English](https://github.com/junkawahara/dd_documents/blob/main/formats/tdzdd_graph_en.md) or [Japanese](https://github.com/junkawahara/dd_documents/blob/main/formats/tdzdd_graph_ja.md) for detail.

If you run

```
./program --path --show grid3x3.txt
```

You will get a ZDD representing all the s-t paths.

The frontier-based search for single cycles is implemented in the FrontierSingleCycleSpec class
(as a "spec" of TdZdd),
and that for s-t paths is implemeneted in the FrontierSTPathSpec class.

If you run the program without arguments like

```
./program
```

it runs for n x n grid for n = 2,...,10, and you will get the following:

```
n = 2, # of solutions = 1
n = 3, # of solutions = 13
n = 4, # of solutions = 213
n = 5, # of solutions = 9349
n = 6, # of solutions = 1222363
n = 7, # of solutions = 487150371
n = 8, # of solutions = 603841648931
n = 9, # of solutions = 2318527339461265
n = 10, # of solutions = 27359264067916806101
```

This implementation uses the Graph class in the TdZdd library. See the document in [English](https://github.com/junkawahara/dd_documents/blob/main/formats/tdzdd_graph_en.md) or [Japanese](https://github.com/junkawahara/dd_documents/blob/main/formats/tdzdd_graph_ja.md).

## Options

### General options

|Option|Effect|
|------|------|
|`--show`|Show information and error messages.|
|`--dot`|Output the constructed ZDD in the graphviz dot format.|
|`--show-fs`|Show the frontiers of the input graph.|
|`--enum`|Enumerate all the subgraphs.|

### Graph types

|Option|Graph|
|------|------|
|`--path`|s-t paths|
|`--hampath`|Hamiltonian s-t paths|
|`--cycle`|Cycles|
|`--letter_O`|O-shaped graphs (equivalent to cycles)|
|`--hamcycle`|Hamiltonian cycles|
|`--path_m`|s-t Paths (using mate)|
|`--hampath_m`|s-t Hamiltonian paths (using mate)|
|`--cycle_m`|Cycles (using mate)|
|`--hamcycle_m`|Hamiltonian paths (using mate)|
|`--forest`|Forests|
|`--tree`|Trees|
|`--stree`|Spanning trees|
|`--matching`|Machings|
|`--cmatching`|Complete matchings|
|`--letter_I`|I-shaped graphs (equivalent to paths)|
|`--letter_L`|L-shaped graphs (equivalent to paths)|
|`--letter_P`|P-shaped graphs|

Vertices s and t of (Hamiltonian) paths are fixed to be 1 and n (the number of vertices), respectively.

## License

MIT License
