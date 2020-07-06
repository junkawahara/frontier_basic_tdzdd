# frontier_basic_tdzdd

An example implementation of the frontier-based search
using TdZdd (https://github.com/kunisura/TdZdd ).

This program constructs a ZDD representing all the single cycles.

## usage

```
make
./program grid3x3.txt
```

We get the following:

```
Reading "grid3x3.txt" ... done in 0.00s elapsed, 0.00s user, 4MB.
# of vertices = 9
# of edges = 12
FrontierExampleSpec .......... <53> in 0.00s elapsed, 0.00s user, 4MB.
# of ZDD nodes = 53
# of solutions = 13
```

The frontier-based search is implemented in the FrontierExampleSpec
(as a "spec" of TdZdd).
