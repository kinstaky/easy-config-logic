# 快速开始

## 下载

```bash
$ git clone https://github.com/kinstaky/eash-config-logic.git
```



## 编译

可以使用 cmake 或者 make 编译，cmake 更慢，但包含了完整的文档和测试，适合进一步的调试和开发；make 更快，更适合在 zynq7020 的系统中编译并直接使用。

### cmake 编译

在 `easy-config-logic` 项目根目录下

```bash
$ cmake -S . -B build
$ cmake --build build
```

编译后的可执行文件在 `build/standalone` 目录下，主要的可执行文件为 `config`

输入安装命令会将可执行文件复制到 `build/bin` 目录下

```bash
$ cmake --build build --target install
```



### make 编译

在 `easy-config-logic` 项目根目录下

```bash
$ make all
```

编译后的可执行文件在根目录 `standalone`目录下，并且会复制到 `bin` 目录下。使用 `make` 编译，源码和 `.o` 中间文件会混合在一起，并且不会编译 `test` 目录下的测试样例



## 测试

仅支持使用 `cmake` 测试

```bash
$ cmake --build build --target test
```



## 使用

参考[配置语法](config_syntax.md)，编写自己的配置文件，或者使用 `example` 目录下的样例 `example_logic_0.txt`，输入如下命令进行配置

```bash
$ ./config -l example_logic_0.txt
```

注意，如果直接在目录下运行会找不到文件，使用 cmake 构建的可执行文件在 `build/standalone/` 目录下，或者使用 cmake 安装后会在 `build/bin/` 目录下，或者使用 make 构建的在 `/bin` 目录下，需要根据实际情况在相应目录下执行。

现在，FPGA 已经被设置好了， 可以使用示波器、信号发生器或者配合 pixie16 插件来进行测试。可以查看[配置示例](config_examples.md)参考该示例的作用和测试方法。更多的功能参考[进阶使用](advanced.md)。
