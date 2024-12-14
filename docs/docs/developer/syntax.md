# syntax 库

> [!CAUTION]
>
> 还没更新，可能是过期内容，谨慎参考

## 简介

这个库用于对逻辑表达式进行词法分析和语法分析。显然，实际上网上有更好的词法分析和语法分析的库，这里纯粹以学习和实践编译原理为目的而从头开始写。如果没有学习过编译原理，可以先学习一下， 或者简单学习一下[上下文无关文法](https://en.wikipedia.org/wiki/Context-free_grammar)，然后看一下我处理逻辑表达式所使用的的文法，最后直接跳到页尾总结查看处理后生成的语法分析树。



## 词法分析

以我浅薄的编译原理知识来看，词法分析涉及到正则表达式，有穷自动机之类的，但是这里为了图省事，并没有写通用的东西，而只是实现了对**变量**和**运算符**的提取。**变量**指的是以字母或者下划线'\_'开头的包含字母、数字和下划线的字符串。**运算符**指 ( 、) 、& 、| 四个运算符，是本项目中逻辑表达式中仅有的可以出现的运算符。

词法分析类命名为 `Lexer`，通过函数 `Analyse` 将 `std::string` 类型的表达式提取成 `std::vector<std::shared_ptr<Token>>`，其中 `Token` 表示的是**变量**或者**运算符**。提取出的[记号](https://en.wikipedia.org/wiki/Lexical_analysis#Token)（token）串将被传递给语法分析器。

这一部分的代码在 `include/syntax/parser/lexer.h` 和 `src/syntax/parser/lexer.cpp` 中



## 语法分析

这里使用了自底向上的分析方法，实现了最简单的 [SLR 语法分析器](https://en.wikipedia.org/wiki/Simple_LR_parser)。我按照教科书上讲的方法，利用[上下文无关文法](https://en.wikipedia.org/wiki/Context-free_grammar)来分析。我定义逻辑表达式的文法如下

0. S -> E
1. E -> E | T
2. E -> E & T
3. E -> T
4. T -> ( E )
5. T -> id

[产生式](https://en.wikipedia.org/wiki/Production_(computer_science))0是起始产生式，产生式1表示逻辑或，产生式2表示逻辑与，产生式3终结 E 的递归，产生式4表示优先运算，产生式5终结 T 的递归。其中 S、E、T 都是[非终结符号](https://en.wikipedia.org/wiki/Terminal_and_nonterminal_symbols)，词法分析一节提到的四个运算符还有 id 都是[终结符号](https://en.wikipedia.org/wiki/Terminal_and_nonterminal_symbols)，其中 id 是变量标识符（用于表示变量）。这里 S 的命名来自于单词 start，E 和 T 则是直接抄教科书中四则运算中的 E 和 T。但是实际上四则运算中 E 应该表示 Expression，T 表示 Term， 还有 F 表示 Factor，是有含义的，我这里属于是生搬硬套了。

简单理解，这个文法所构成的“逻辑语言”

+ 基本元素是变量和运算符
+ 二目运算符 | 和 & ，优先级相同
+ 单目运算符 ()，优先级最高

总结一下前面提到的关键词，这样的逻辑语法自底向上，构成的元素为 记号（token）、产生式（production）、语法（grammar）。因此，我构建了 `Token`、`Production` 和 `Grammar` 类来表示特定的语言（当然这个项目中主要表示的是逻辑表达式的语言），并构建类 `SLRSyntaxParser` 作为语法分析器。



### Token

`Token`的定义和实现在 `include/syntax/parser/token.h` 和 `src/syntax/parser/token.cpp`。在我看来 `Token` 就是一个占位符，作为一个基本元素构成更上层的产生式。

这里仅将标记分为两类

+ `Identifier` 标识符表示变量
+ `Operator` 运算符

两者都是 `Token` 的子类。考虑到可能需要对变量进行求值，`Attach` 可以令标识符附加一个 C++ 内的变量，并通过 `GetAttached` 得到变量的值（即标识符此时的值）。而 `Operator` 几乎就是一个占位符。



### Production

`Production` 的定义和实现在 `include/syntax/parser/production.h` 和 `src/syntax/parser/production.cpp` 中。`Production` 顾名思义，是会产生东西的。具体在语法分析过程中，可以由抽象语法产生具体语法，抽象语法就是语法分析中提到的那六个产生式，具体语法就是对应于具体的变量和运算符的逻辑表达式的一部分或全部。比如说，表达式 `A | B` 就是一个具体语法，对应于抽象语法 `E | T`。所以，我个人觉得这里很适合使用工厂模式，抽象语法作为工厂`ProductionFactory`，具体语法就是产品`Production`。

另一方面，可以将产生式看成树，产生式左边的非终结符号是父节点，产生式右边的非终结符号和终结符号是子节点。因此，我又添加了一个类 `ProductionFactorySet` 来表示非终结符号，命名为 set 是因为往往一个非终结符号有多种产生式（即作为多个产生式的左端）。由于非终结符号和终结符号（`Token`）都是节点，所以我再添加了一个类`Symbol`作为它们的基类，使得可以使用 `Symbol*` 来代表语法树的节点。

由于后面我将使用 SLR 语法分析器，所以还需要从产生式中构建**项**（item）和**项集**（collection），所以我又添加了两个类`ProductionItem` 和 `ProductionItemCollection` 来表示项和项集。

这样，就有5个和产生式相关的类了。此时可以发现多个类有共同的结构和方法，所以我给`Production`、`ProductionFactory` 和 `ProductionItem` 添加了共同的基类 `ProductionBase` 以减少代码的重复。

#### 小结

`ProductionBase` 主要包含了对父节点和子节点操作的方法。

`Production` 包含利用动作（action）对产生式求值（Eval）的方法。

`ProductionFactory` 可以产生 `Production` 和构建项 `ProductionItem`。

`ProductionFactorySet` 表示非终结符号，和 `ProductionFactory` 之间的关系有点复杂，一个 `ProductionFactorySet` 包含多个 `ProductionFactory` 同时，`ProductionFactory` 将 `ProductionFactorySet` 作为子节点。

`ProductionItem` 由 `ProductionFactorySet` 产生，在 `ProductionBase` 基础上增加了一个变量表示是其原型 `ProductionFactorySet` 的第几项，以在 SLR 语法分析器中使用。

`ProductionItemCollection` 是项 `ProductionItem` 的集合，其内部区分了核心项（core items）和非核心项（non-core items），并带有 goto 表，都是为了方便在 SLR 语法分析器中使用。



### Grammar

`Grammar` 的定义和实现在 `include/syntax/parser/grammar.h` 和 `src/syntax/parser/grammar.cpp` 中。

多个抽象产生式 `ProductionFactory` 构成了特定的 `Grammar`，即语法或者语言。语法的直接构成元素是产生式，基本的构成元素是终结符号和非终结符号，所以该类会记录所有产生式中的符号到符号表中，使用方法 `SymbolList` 可以得到符号表。

考虑到后续使用 SLR 语法分析器时，需要构造项集和行为表（action table），所以我在 `Grammar` 中增加了更多的方法以方便构造。

使用 SLR 语法分析器时，第一步是构建项集，所以我增加了方法 `GenerateCollection`；构建项集的过程需要用到闭包函数，因此我添加了方法 `MakeClosure`；还有 `NextItem` 查找特定项对应的下一项。同时其内部构建了项集的 goto 表，方法 `CollectionGoto` 可以产看项集之间的转换。

使用 SLR 语法分析器的第二步是构建行为表，使用到了函数 First 和 Following 函数（实际上 LL 语法分析器也会用到这两个函数）；因此我又添加了方法 `First` 和 `Following` 返回对应的串，如果对应的串不存在，它们会分别调用方法 `GenerateFirst` 和 `GenerateFollowing` 来构建 First 和 Following 串。另一方面，产生式可能产生空串（$\epsilon$），空符号（串）在这里没有使用特殊的符号来表示，而是利用方法 `FirstIncludeEmpty` 来表示 First 串中可能包含空。

对于特定的语法，我认为比较好的构建方法是作为 `Grammar` 的继承类。这里给出了两个例子，表示逻辑表达式的 `LogicalGrammar` 和四则运算的 `ArithmeticGrammar`。我定义的这些继承类都只包含构造函数和析构函数，在构造函数中完成语言的构建。我构建的流程如下

+ 定义终结符号
    + 使用 new 定义标识符，标识符只需要一个，这里利用标识符的基类 `Symbol` 实现，并设置类型变量为标识符类型，以此来表示是一个通用的标识符
    + 使用 new 定义运算符
+ 定义非终结符号，即 `ProductionFactorySet`，使用 new 定义
+ 定义抽象产生式，即 `ProductionFactory`，并给其添加拓展的行为（action）用于求值（evaluate），使用 new 定义
+ 将抽象产生式添加到非终结符号中，即通过调用 `ProductionFactorySet::AddProductionFactory` 令 `ProductionFactorySet` 包含 `ProductionFactory`
+ 将终结符号和非终结符号添加到抽象产生式中作为其子节点，通过调用 `ProductionFactory::SetChildren`
+ 通过将 `ProductionFactorySet` 添加到 `Grammar` 中将上面的所有定义的符号、产生式添加到 `Grammar` 中，通过调用 `AddProductionSet`添加
+ 将所有 new 出来的对象添加到串中，以便在析构函数中删除

使用这套模板，我构建了两种语法

#### LogicalGrammar

定义和实现在 `include/syntax/logical_grammar.h` 和 `src/syntax/logical_grammar.cpp`

其文法如下

0. S -> E
1. E -> E | T
2. E -> E & T
3. E -> T
4. T -> ( E )
5. T -> id

用于表示逻辑表达式，也是本项目后续使用的语法。



#### ArithmeticGrammar

定义和实现在 `include/syntax/arithmetic_grammar.h` 和 `src/syntax/arithmetic_grammar.cpp`

其文法如下

0. S -> E
1. E -> E + T
2. E -> E - T
3. E -> T
4. T -> T * F
5. T -> T / F
6. T -> F
7. F -> ( E )
8. F -> id

用于表示四则运算，目前仅在测试中使用，因为没有配套的词法分析。



### SLRSyntaxParser

定义和实现在 `include/syntax/parser/syntax_parser.h` 和 `src/syntax/parser/syntax_parser.cpp`

`SLRSyntaxParser` 的主要功能函数 `Parse` 读入从`Lexer` 传递的  `std::vector<TokenPtr>`，然后构建语法分析树。构建后，可以从方法 `Root()`得到语法分析树的根节点，也可以通过方法 `PrintTree` 将其带结构地输出；如果已经给表达式中的变量附上 C++ 变量（就是赋值），还可以使用方法 `Eval` 求值。

为了可以实现 `Parse` 的功能，我按照书上的 SLR 语法分析器的算法，如 [Grammar](#grammar) 一节中所说，先构建项集，然后构建行为表，这两部都写在了构造函数中。由于没有提前定义表示结束的符号 \$，这里认为结束符号就是符号表中最后一个符号，它的序号等于符号表的长度，所以它是一个特殊的符号，需要小心地处理。我觉得更合理的可能是定义一个特殊的符号来代表结束符号，而不是像现在这样用特殊的方式处理。除此之外，我觉得这里比较复杂的是`Parse` 函数中  `itoken` 和 `look_symbol` 两个变量。`itoken` 用来指示语法分析器处理到了输入的记号串的位置，用以表示书中的队列。`look_symbol` 则是表示当前在看的符号，可能是输入的记号，也可能规约后的非终结符号，同时还是在行为表中查找当前需要执行的行为的依据。



#### ActionTable

为了方便写入和查找行为表，我添加了类 `ActionTable`，定义和实现都与 `SLRSyntaxParser` 在同一文件中。

我们知道在 SLR 语法分析器中，主要有4种行为

+ 移入（shift）
+ 规约（reduce）
+ 接受（accept）
+ 错误（error）

行为指导了分析器如何处理看到的符号，并如何在（作为状态的）项集之间转化。实际上本项目中处理复杂的只有移入和规约行为，接受就相当于完成分析，错误在本项目中简单处理，只管报错，直接中断，虽然对用户不友好，但是足够简单。

因此，我定义的结构 `Action` 主要考虑了移入和规约两种行为，仅包含行为类型 `type`，转换的项集 `collection` 和规约成的产生式 `production`。通过方法 `SetAction` 向行为表中写入行为，通过方法`GetAction` 读取行为。

至于行为表内部实际使用一维数组来存储二维的表，所以会有手动转换下标的部分。



### 小结

以上就是语法分析器的全部，从语法的基本元素开始，包括了 Token、Production、Grammar，由 SLRSyntaxParser 总领，以将读入的记号串转换成语法分析树。



## 比较逻辑表达式

为了进一步验证前面工作的正确性，我另外添加了类 `LogicComparer` 来比较两个逻辑表达式是不是等价的。所谓等价，即两者的真值表相同，或者说形式等价。比如说 A & B 和 A | B 是不一样的，但是 （A & B）| C 和 （A | C) & (B | C) 是等价的。这一部分的定义和实现在 `syntax/logic_comparer.h` 和 `syntax/logic_comparer.cpp`。

一个朴素的想法就是比较两个表达式的真值表，这对于人类难说难以完成，但对于机器来说恰到好处。所以，可以通过遍历所有逻辑变量的值，然后比较两个逻辑表达式的输出来比较真值表。我也是如此做的。

当然，对于一些简单的不到10个变量的表达式来说，遍历还是很快的，但我们知道遍历的时间时指数增长的，变量数量增加后机器也很快不堪重负。所以需要小心地设置遍历方法并引入缓存。

在遍历的过程中，如果使用[格雷码](https://en.wikipedia.org/wiki/Gray_code)，那么每次只有一个变量在变化，只有受这个变量影响的部分表达式的值发生了变化。比如说，对于表达式 (A & B) | C，如果变化的是C，那么 A & B 的值不受影响；但如果变化的是 A，那么无论是 A & B 还是 （A & B) | C 的值都发生了变化。不受影响的值就不需要重新计算，可以使用上一次的值，这就是缓存。

另一方面，当然是希望某个变量改变后，受到影响的值更少，像前面的例子中，我们更希望变化的是 C 而不是 A。恰好在遍历的过程中，有的值频繁变化，比如格雷码最低位每两次遍历就变化一次；有的值变化很少，比如最高位，整个遍历过程只改变一次。所以可以让影响小的变量处于格雷码的低位，而影响大的处于格雷码的高位。

而如何评估一个变量的影响大小呢？简单起见，我直接把变量在语法分析树中的深度作为衡量影响大小的唯一依据，虽然不严谨，但是也足够使用了。一个变量在语法分析树中的深度越深，其影响就越大；相反，深度越浅，影响越小。

如此，遍历一个表达式的值的时间变得可以接受，即使是20个变量的表达式也能在 1s 内完成遍历。

另外，可以运行程序 `compare`可以比较两个逻辑表达式是否等价。

## 总结

这个库的主要目的，就是使用编译原理的方法，令机器看懂逻辑表达式。词法分析器读入字符串类型的逻辑表达式，处理成记号串后传递给语法分析器，语法分析器再根据产生语法分析树。所谓语法分析树，就是将字符串转换成树状结构，以直观表现使用文法推导出出串的过程。

运行 `syntax_tree` 程序可以看到最终产生的语法分析树。

```bash
$ ./bin/syntax_tree
(var0 & var1) | var2
S1
├─S1
│ └─S2
│   ├─(
│   ├─S1
│   │ ├─S1
│   │ │ └─S2
│   │ │   └─var0
│   │ ├─&
│   │ └─S2
│   │   └─var1
│   └─)
├─|
└─S2
  └─var2
```

S1 表示文法中的 E，S2表示文法中的 T（还有S0 表示的 S，但这里省略了）。可以从语法分析树直观地看出从产生式到字符串的推导过程：

+ E0 -> E1 | T0
+ E1 -> E2 & T1
    + E2 -> T2 -> var0 (id)
    + T1 -> var1 (id)
+ T0 -> var2 (id)

下标 0、1、2 用于标识不同的 E 和 T。

如此处理后，后续可以很轻易地知道输入的逻辑表达式的含义，并在语法分析树的基础上进一步处理。

