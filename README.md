# easy config logic

This project aims to config the trigger logic in some nuclear experiments easily using the logic hardware [MZTIO](https://xia.com/products/mz-trigio/) from [xia](https://xia.com).


> **Warning**
> This project is used for my own learning and experimention, so there may be a great deal that does not make sense. If this project is barely able to run, that's enough. So most of the content in this project was only optimized to run in limited time(less than 1 second) or not optimized at all(it cost less than 1 second at the beginning), and no cosideration is was given to performance, compatibility and robustness. The author has only a little experience in C++.


## download and install
In the embedded Linux in MZTIO
```bash
git clone https://github.com/kinstaky/easy-config-logic.git
```

Then build with make or cmake. It's recommended to build with make in MZTIO since it's faster.

### build with make
```bash
cd easy-config-logic
make all
```

### build with cmake
```bash
cd easy-config-logic
cmake -S . -B build
cmake --build build
cmake --build build --target install
```

## test
Only support using ctest
```bash
cmake --build build --target test
```

## run example
Suppose build with make
```bash
./bin/config -l examples/example_logic_0.txt
```
For more information, read the full document: https://kinstaky.github.io/easy-config-logic-doc/

