# MZTIO 逻辑插件

## 插件简介

[MZTIO](https://xia.com/products/mz-trigio/) 是由 xia 生产的基于 xilinx 的 [zynq7000](https://www.xilinx.com/products/silicon-devices/soc/zynq-7000.html) 系列 FPGA 芯片的逻辑插件。在和 [pixie16](pixie16.md) 获取插件配套使用时，主要使用其

+ 前面板 64 路网口 [LVDS](https://en.wikipedia.org/wiki/Low-voltage_differential_signaling) 输入输出
+ 子板可拓展的8、16、24路 [LEMO](https://en.wikipedia.org/wiki/LEMO) 口 [TTL](https://en.wikipedia.org/wiki/Transistor%E2%80%93transistor_logic) 输入输出
+ 背板单路输入输出。

官方说明书可以访问 https://s3.us-west-1.amazonaws.com/download.xia.com/products/mztio/documents/manuals/XIA-Pixie-16-MZ-TrigIO-MAN-Users-Manual-2019-12-12.pdf 获得。



![mztio_apperance](https://xia.com/wp-content/uploads/xia-mztio-iso-l-transparent-sq.png)

## 前面板输入输出

如图前面板左边，一共有13个网口，其中最下面一个标注为 Net，意为用于网络连接，而其他12个网口则是用于和 pixie16 插件进行逻辑信号传输。使用时只需要使用网线连接 pixie16 的网口和 MZTIO 插件的任意网口，再正确地配置，即完成了连接。为了方便后续配置，这12个网口将被分成从上到下的 A、B、C 3组，每组4个网口。由于每个网口中有4路输入输出，所以每组有16路输入输出，总计48路输入输出。同样为了方便，这48路从上到下命名为A0-A15，B0-B15，C0-C15；即第一个网口中的4路信号命名为A0、A1、A2、A3，第二个网口中的4路信号被命名为A4、A5、A6、A7……如此类推，即第十二个网口的4路信号命名为C12、C13、C14、C15。

该网口的标准是 rj45，通过搜索引擎可以得到更多的信息。



## 子板 LEMO 输入输出

[官方说明书](https://s3.us-west-1.amazonaws.com/download.xia.com/products/mztio/documents/manuals/XIA-Pixie-16-MZ-TrigIO-MAN-Users-Manual-2019-12-12.pdf)第13章简要描述了子板的作用。一块逻辑插件可以安装 0-3块子板，每一块子板对应一组网口（A或B或C），每块子板上有8个 LEMO 口，每个 LEMO 同一时间只能作为输入或者输出，但是可以通过调节子板上的跳针更改为输入或者输出。 LEMO 口和网口是并行的，子板上的 LEMO 口每2个对应1个网口，如上图插在 B 组旁边的子板对应 B 组网口，8个 LEMO 口从上到下分别对应网口线路 B1，B2，B5，B6，B9，B10，B12，B13；因此，这8个 LEMO 口分别被命名为 B17，B18，B21，B22，B25，B26，B29，B30，即对应的网口那一路名字的数字加16，以表示对应关系。

因为存在对应关系，所以对应的网口那一路和 LEMO 口该路不能同时为输入，不然会导致内部矛盾，除此之外的输入输出组合都是可行的，即

+ LEMO 口和网口对应路同时为输出，且两个输出信号在物理意义上一样（即同时表示 true 或者 false，但是要注意两个信号的标准不一样）
+ LEMO 口作为输入，对应网口输出和 LEMO 口一致的信号
+ 网口对应路作为输入，LEMO口输出和网口对应路一致的信号

注意到一块子板的  LEMO 口只有8个，而对应的4个网口一共有16路信号，所以只有一半的网口线路有对应的 LEMO 口。对于一个网口而言，比如说 B 组第一个网口，其4路分别是 B0、B1、B2、B3，只有 B1 和 B2 有对应的 LEMO 口，主要是考虑到假如网线另一端插在 [pixie16](pixie16.md) 插件中时，B0 和 B3 将作为 pixie16 插件的输出即 MZTIO 插件的输入，所以在 LEMO 口有限的情况下这两路就不设置对应路了。 

## 背板输入输出

由于本人才疏学浅，对于背板信号的只摸索出了单路可用的信号，该信号可以在 [pixie16说明书](https://s3.us-west-1.amazonaws.com/download.xia.com/products/pixie16/documents/manuals/XIA-Pixie16-Users-Manual-3.06-2019-09-13.pdf)3.3.11.5 小节中被称为 **ET_LocalCrate_BP**信号，属于 Module Validation Trigger 的一种。该信号是一个广播信号，一个机箱中只有一个插件（pixie16 或者 MZTIO）可以输出该路信号，否则可能产生冲突；同时机箱中所有的插件（在本项目中 MZTIO 不考虑接收该信号，但脱离了本项目可以接收并处理该信号）都可以接收这个信号，而 pixie16 插件可以将接受的该信号作为其 Module Validation Trigger 的来源。

在本项目中，该单路背板信号被命名为 Back，同时只允许作为输出信号。
