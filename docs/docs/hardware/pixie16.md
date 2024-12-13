# pixie16 插件

## 简介

官方说明书可以访问 https://s3.us-west-1.amazonaws.com/download.xia.com/products/pixie16/documents/manuals/XIA-Pixie16-Users-Manual-3.06-2019-09-13.pdf 获得，这里并不会详细说明如何使用 pixie16 插件获取数据，而只会简略说明设置逻辑部分。注意：本项目使用固件版本为 pixie16-refvpku-release-06092020。



## 输入输出

对于pixie16插件而言，输入和输出的端口为前面板的网口和背板。其中网口包含2路输出和2两路输入，背板是单路的输出或者输入。

### 输出

使用 pku 版本的固件时，网口的第1路和第4路为输出，输出的是[说明书](https://s3.us-west-1.amazonaws.com/download.xia.com/products/pixie16/documents/manuals/XIA-Pixie16-Users-Manual-3.06-2019-09-13.pdf)3.3.11 一节中描述的 Multiplicity 或者 Coincidence 触发，其中参数 Multiplicity Mask High[16] 这一位决定该路是 Multiplicity 还是 Coincidence 触发。网口的第4路输出通道0的 Multiplicity/Coincidence 触发，第1路输出通道1的 Multiplicity/Coincidence 触发，该输出不受其他参数影响，即无需考虑 Channel Validation Trigger 的设置。至于具体的 Multiplicity/Coincidence 触发的设置，详见[说明书](https://s3.us-west-1.amazonaws.com/download.xia.com/products/pixie16/documents/manuals/XIA-Pixie16-Users-Manual-3.06-2019-09-13.pdf)3.3.11.1 和 3.3.11.2 小节。

至于背板，[说明书](https://s3.us-west-1.amazonaws.com/download.xia.com/products/pixie16/documents/manuals/XIA-Pixie16-Users-Manual-3.06-2019-09-13.pdf)3.3.11.5 和 3.3.11.6 两小节，可知只有设置了 ModCSRB[6]=1 时，该插件才能向其所在的单个机箱广播它的 Ext_ValidTrig_In 和 Ext_FastTrig_In 信号分别作为其它插件的可能的 Module Validation Trigger 和 Module Fast Trigger；或者设置了 ModCSRB[4]=1 时，该插件才能向系统内所有机箱广播上述两个信号。但是，本项目只考虑使用 Module Validation Trigger 的情况，所以将背板看成单路的输出。

### 输入

使用 pku 版本的固件时，网口的第2路和第3路作为输入，参考[说明书](https://s3.us-west-1.amazonaws.com/download.xia.com/products/pixie16/documents/manuals/XIA-Pixie16-Users-Manual-3.06-2019-09-13.pdf)3.3.11.5 和 3.3.11.6 小节，第3路即是 Ext_ValidTrig_In 4个来源中的 LVDS_ValidTrig_FP，第2路是 Ext_FastTrig_In 4个来源中的 LVDS_FastTrig_FP。即网口的输入可以作为该插件的 Module Validation Trigger 和 Module Fast Trigger。

至于背板，同样是 3.3.11.5 和 3.3.11.6 小节说明了背板可以接收 ET_LocalCrate_BP 信号作为 Module Validation Trigger；接收 FT_LocalCrate_BP 信号作为 Module Fast Trigger。但是同样的，本项目中不考虑使用 FT_LocalCrate_BP 信号，所以可以看成背板只有单路的输入。

### 小结

对于 pixie16 插件，网口的输入输出为

+ 第1路，输出，通道1的 Multiplicity/Coincidence 触发
+ 第2路，输入，Module Fast Trigger 的可用来源
+ 第3路，输出，Module Validation Trigger 的可用来源
+ 第4路，输出，通道0的 Multiplicity/Coincidence 触发

背板在本项目中相当于单路的输入或者输出

+ 1个机箱中最多只有1个插件可以向背板输出
+ 所有机箱中也最多只有1个插件可以向所有机箱的背板输出
+ 其他插件可以将背板输入作为可用的 Module Validation Trigger 来源