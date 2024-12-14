# 配置 MZTIO 插件

## 引入

本节简要介绍怎么配置 MZTIO 插件。MZTIO 插件的核心是 Microzed 的红色板子，而红色板子的芯片是 Xilinx 的 zynq7020 芯片。该芯片融合了 FPGA 芯片和 Cortex-A9 CPU，兼容了 FPGA 和 CPU 的优点。因此可以把 FPGA 看成是 CPU 的延伸，相当于在一台微型电脑（嵌入机）中插入了一些外设。

我们控制 FPGA 基本是通过嵌入机来完成。所以我们从配置其嵌入机的操作系统开始配置 MZTIO 插件，然后再安装本项目的程序。

## 安装操作系统

本项目依赖 [xillybus](http://xillybus.com/xillinux) 提供的 xillinux，实际是 Ubuntu16.04 的修改版。

> [!NOTE]
>
> 本项目不限于在嵌入机中使用此 xillinux，但并未考虑其他系统的兼容性。

详细阅读  提供的文档 [Getting started with Xillinux for Zynq-7000](http://xillybus.com/downloads/doc/xillybus_getting_started_zynq.pdf)，包括了如何下载和安装系统。下面简要介绍步骤。

下载 [xillybus](http://xillybus.com/xillinux) 提供的 linux 系统（下载地址 http://xillybus.com/downloads/xillinux-2.0a.img.gz ），然后写入到 SD 卡中作为操作系统（比如使用 [rufus](https://rufus.ie/en/)），就像制作 U 盘启动盘一样。

写入完成后，SD 卡会被分为两个分区

1. 启动分区（boot），里面的文件是开机的时候用的，目前里面只有一个 uImage 文件
2. 文件系统分区（rootfs），是操作系统的根目录，里面能够看到平常在 linux 根目录看到的文件，比如说 home，sys

对于一般的电脑来说，操作系统已经安装好了，但是对于嵌入机来说，还需要考虑对应的外设。所以还要再复制三个文件到启动分区中。从本项目的 Release 页面中下载启动文件，https://github.com/kinstaky/easy-config-logic/releases/download/v2.3.0/bootfile.zip。下载后解压，能够得到以下三个文件，都复制到启动分区中

1. boot.bin
2. devicetree.dtb
3. easy_config_logic_v_2_3.bit

其中，把 easy_config_logic_v_2_3.bit 改名为 xillydemo.bit。至此，启动分区中一共有 4 个文件。操作系统的安装和配置也就完成了。

> [!TIP]
>
> 也可以从http://xillybus.com/downloads/xillinux-eval-microzed-2.0d.zip  下载 xillybus 提供的启动文件。下载解压后，在 bootfiles 文件夹中找到 boot.bin 和 devicetree.dtb 两个文件。但要注意 xillybus 提供的 xillydemo.bit 和本项目并不适配，所以请使用本项目提供的 easy_config_logic_v_2_3.bit。

## 启动系统

1. 将制作好的 SD 卡插入到 MZTIO 上的 zynq-7020 板子的插槽中
2. 使用网线连接 SD 卡插槽旁边的网口（本项目暂不支持前面板网口）和路由器
3. 将 MZTIO 模块插入到 xia 的机箱中
4. 启动 xia 机箱
5. 使用 ssh 连接系统

## 安装软件

### 安装服务端

下载 https://github.com/kinstaky/easy-config-logic/releases/download/v2.3.0/ecl-bin.tar，然后解压到嵌入机中，就算安装完成了。

里面包含以下可执行文件

+ compare，本项目测试工具，用于比较两个逻辑表达式是否等价
+ config，通过配置文件配置 MZTIO 插件，无 GUI 界面
+ convert，用于将逻辑表达式形式配置文件转化为寄存器形式配置文件
+ server，用于配合客户端 GUI 界面，**本项目招牌菜**
+ standardize，本项目核心工具，用于将逻辑表达式“标准化”
+ syntax_tree，本项目核心工具，用于解析逻辑表达式

里面实际上只有 config、convert、server（甚至只有 server）是需要的，所以也可以在 [本项目 Release 页面](https://github.com/kinstaky/easy-config-logic/releases/) 按需下载可执行文件。

### 安装客户端

客户端安装在平常使用的 Linux 电脑中，一般是实验的时候放在获取室的电脑。

> [!NOTE]
>
> 客户端仅在 Ubuntu 20 和 Ubuntu 22 中测试过

从https://github.com/kinstaky/easy-config-logic/releases/download/v2.3.0/easy_config_logic_client_1.0.0.deb 下载安装包，双击安装包安装。

也可以通过以下命令安装。

```bash
sudo dpkg -i easy_config_logic_client_1.0.0.deb
```

安装后通过命令运行客户端。该客户端是带有 UI 界面的，应该比较容易使用吧。

```bash
easy_config_logic_client
```

## 更多信息

阅读 [The guide to Xillybus Lite](http://xillybus.com/downloads/doc/xillybus_lite.pdf) 以了解更多关于 xillinux 和 xillybus lite 的内容。

阅读 [编译](../developer/compile.md) 一节以了解如何自己编译本项目并得到对应的可执行程序。

阅读 [客户端](../developer/client.md)一节以了解如何自己编译本项目对应的客户端程序。