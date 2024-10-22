# cyk-parser

A rudimentary implementation of both a CFG-to-CNF converter and CYK parser written in C++.

## Building

```sh
mkdir build
cd build

cmake ..
make -j4
```

## Running

This project creates two outputs;

1. `CYK_Parser` - A shared library containing necessary classes/functions to perform grammar checking.
2. `CYK_Tests` - A simple executable which links to `CYK_Parser` and checks a number of conforming and non-conforming sentances against different grammars.

##