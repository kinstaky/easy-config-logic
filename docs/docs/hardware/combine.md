# 结合使用 pixie16 和 MZTIO 插件

## 简介

本节简单介绍如何将 pixie16 插件和 MZTIO 插件结合起来使用。注意，只是本人可能会如此使用，并不代表只能如此使用，实际仍需结合实际情况调整。

结合使用的前提是单个、两个或者三个 pixie16 插件的内部逻辑不足以支撑实验需求，所以需要外部插件 MZTIO 来配置更复杂的逻辑。



## 单个机箱

所有的 pixie16 插件都只通过网线输出，可能只输出1路，也可能是2路都输出。MZTIO从网口接收所有来自 pixie16 的输出，在内部进行逻辑组合后，对于需要外部触发的 pixie16 插件

+ 如果这些 pixie16 插件需要同一种总触发，MZTIO 直接通过背板输出作为所有 pixie16 插件的 Module Validation Trigger，同时 pixie16 设置成需要 Module Validation Trigger 且来源为 ET_LocalCrate_BP
+ 如果某些 pixie16 插件需要区别于总触发的触发，MZTIO 直接通过网线的第3路给该插件输入所需要的触发，同时该 pixie16 插件设置成需要 Module Validation Trigger 且来源为 Ext_ValidTrig_In 的 LVDS_ValidTrig_FP

使用这样的方式的好处是如果大部分 pixie16 插件的 Module Validation Trigger 都来自于背板，则延迟时间和逻辑门宽的设置是相似的。



## 多个机箱

同样的，所有的 pixie16 插件都只通过网线输出。MZTIO从网口接收所有来自 pixie16 的输出，在内部进行逻辑组合后，将总触发通过网线传递给每个机箱的主管插件（即参数 ModCSRB[6] 设置为1的插件）。

+ 所有需要外部触发的插件设置为需要 Module Validation Trigger
+ 主管插件的 Module Validation Trigger 来源设置为 Ext_ValidTrig_In 的 LVDS_ValidTrig_FP
+ 其他需要总触发的插件的 Module Validation Trigger 来源设置为 ET_LocalCrate_BP，实际是有主管插件发出的
+ 需要区别于总触发的触发的插件从网口接收 MZTIO 从网线传递的信号，Module Validation Trigger 来源设置为 Ext_ValidTrig_In 的 LVDS_ValidTrig_FP

使用这样的方式的好处是因为每个机箱的结构是相似的，所以不同机箱的主管插件的参数设置是相似的，不同机箱的普通插件的参数设置也是相似的。



## 小结

本节所介绍的设置方法，其目的在于保证不同插件的地位的相似性

+ 在单机箱中，不设置主管插件，令每个插件地位平等
+ 在多机箱中，考虑到 MZTIO 网口可能不够用，只能再每个机箱中设置主管插件，但是每个机箱的地位平等

如此，以期更多的插件保持相似性，以使用相似的延迟和宽度参数。

