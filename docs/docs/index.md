# easy-config-logic

## 简介

本项目用于更方便地设置基于 [xia](https://xia.com) 获取和 [MZTIO](https://xia.com/products/mz-trigio/) 逻辑插件的数字化获取系统的核物理实验触发逻辑，力求摆脱硬件细节，让用户专注于实验逻辑和物理目标。

项目地址：https://github.com/kinstaky/easy-config-logic/

## 文档结构

第一章为硬件基础，第二章为使用指南，第三章为开发指南。

阅读提示

+ 如果只需要在实验值班中看看计数率，阅读 [开始使用](user_guide/getting_started.md)

+ 如果需要调节逻辑，但不需要配置系统，直接阅读第二章的[配置语法](user_guide/config_syntax.md)、[配置示例](user_guide/config_examples.md)。

+ 如果需要从头配置系统，阅读[配置 MZTIO 插件](user_guide/setup_mztio.md)
+ 如果不熟悉硬件，阅读[第一章](hardware/mztio.md)
+ 如果对项目的实现感兴趣，阅读[第三章](developer/syntax.md)



## 帮助改进

欢迎提交 [issue](https://github.com/kinstaky/easy-config-logic-doc/issues) 对本说明文档提出修改意见，也欢迎对本项目 easy-config-logic 有疑惑的同志到项目地址中提交 [issue](https://github.com/kinstaky/easy-config-logic/issues)，或者发送邮件到 kinstaky@163.com。

> [!NOTE]
>
> 本项目除了简介所述目的，另一用途为本人学习和试验，本人水平有限，可能存在大量不够合理之处，请小心使用。对于该项目，目标是勉强能用，大部分内容仅优化到能在有限时间（小于1秒）内跑完，也没有考虑性能表现、兼容性和鲁棒性。