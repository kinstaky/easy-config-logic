# 开始之前

## 简介

本节简要介绍了如何安装 xillinux 系统到 MZTIO 的系统中。如果已经安装好系统，跳到[下一节](quick_start.md)。安装好后输入 `uname -r` 会显示如下

```bash
$ uanme -r
4.4.30-xillinux-2.0
```

本项目不限于在嵌入机中使用此操作系统，但并未考虑其他系统的兼容性。

## 安装系统

详细阅读 [xillybus](http://xillybus.com/xillinux) 提供的文档 [Getting started with Xillinux for Zynq-7000](http://xillybus.com/downloads/doc/xillybus_getting_started_zynq.pdf)，包括了如何下载和安装系统。下面简要介绍步骤。

下载 [xillybus](http://xillybus.com/xillinux) 提供的 linux 系统（下载地址 http://xillybus.com/downloads/xillinux-2.0.img.gz） ，然后写入到 sd 卡中作为操作系统（比如使用 [rufus](https://rufus.ie/en/)），就像制作 U 盘启动盘一样。

下载 [xillybus](http://xillybus.com/xillinux) 提供的启动文件（下载地址 http://xillybus.com/downloads/xillinux-eval-microzed-2.0d.zip ）。下载解压后，打开 bootfiles 文件夹，复制文件 boot.bin 和 devicetree.dtb 到 SD 卡的 boot 分区中（制作好 SD 卡启动后，SD 卡内会有两个分区，一个是 boot 分区，里面有 uImage 文件，另一个是根目录，会有平常 linux 系统根目录下的各种文件夹，比如 sys，home）。



## 复制 bitstream 文件

xillybus 的教程中指导了如何生成自己的 bitstream 文件，但本项目中已经预先生成了适用于 MZTIO 插件的 bitstream 文件。复制本项目 examples/fpga 目录下的 general_logic_v2.bit 文件到 SD 卡的 boot 分区中并命名为 xillydemo.bit。至此，boot 分区下应该有以下 4 个文件

1. uImage
2. boot.bin
3. devicetree.dtb
4. xillydemo.bit

## 启动系统

1. 将制作好的 SD 卡插入到 MZTIO 上的 zynq-7020 板子的插槽中
2. 使用网线连接 SD 卡插槽旁边的网口（本项目暂不支持前面板网口）和路由器
3. 将 MZTIO 模块插入到 xia 的机箱中
4. 启动 xia 机箱
5. 使用 ssh 连接系统

## 更多信息

阅读 [The guide to Xillybus Lite](http://xillybus.com/downloads/doc/xillybus_lite.pdf) 以了解更多。