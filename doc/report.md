# Compiler

xxx, xxx, xxx

## 介绍

## 词法分析

使用Lex作为词法分析工具，词法分析见```lex.l```，参考了ANSI C的Lex[1]。

## 语法分析

## 语义分析

## ...

## 测试

### AST可视化

我们编写了AST转json的代码，可以将抽象语法树转化为json。使用d3.js来实现json的树形可视化。
具体方法是将生成data.json文件复制到d3文件夹下，而后在d3文件夹下控制台输入```python -m http.server```（要求安装python3），启动HTTP服务器。
之后在浏览器中访问```localhost:8000```即可查看可视化的抽象语法树。这个抽象语法树是可以交互的，可以实现语法树节点的展开收起。
d3.js的代码参考了[4].

例如如下C语言代码，经过我们的编译器生成AST后，进行可视化，效果如图所示.

```C
```

![AST Visualization]()

## 参考资料

[1] http://www.quut.com/c/ANSI-C-grammar-l-1998.html

[2] http://www.quut.com/c/ANSI-C-grammar-y-1998.html

[3] https://docs.microsoft.com/en-us/cpp/c-language/c-language-reference?view=vs-2019

[4] http://bl.ocks.org/robschmuecker/7880033
