# frontier_basic_tdzdd

An example implementation of the frontier-based search
using TdZdd (https://github.com/kunisura/TdZdd ).

This program constructs a ZDD representing all the single cycles and a ZDD representing all the s-t paths on a given graph.

## usage

```
make
./program grid3x3.txt
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

If you run

```
./program grid3x3.txt -p
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

This implementation uses the Graph class in the TdZdd library. See the document in [English](https://github.com/junkawahara/documents/blob/master/tdzdd_graph_en.md) or [Japanese](https://github.com/junkawahara/documents/blob/master/tdzdd_graph_ja.md).

