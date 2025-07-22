# 简易编译器前端实现（课设）

> 基于Qt框架实现的C语言子集编译器前端，包含词法分析、语法解析、AST生成及GUI可视化

## 核心功能
1. 词法分析器
  	支持30+ Token类型（关键字/运算符/标识符等）
   	精准错误定位（行号+列号）
2. 递归下降语法解析器
   	解析变量声明、if-else、for循环等结构
生成抽象语法树（AST）
3.可视化界面
实时语法高亮编辑
  	Token表详细展示
 	 AST树形结构展示
 	错误定位源码

## 技术栈
编程语言: C++17
GUI框架: Qt 6.9.1
核心算法:
 	有限状态自动机（词法分析）
 	递归下降法（语法分析）
  	作用域栈+哈希表（符号表管理）

## 界面截图
<img width="461" height="380" alt="image" src="https://github.com/user-attachments/assets/bafa254d-6ca1-4185-83c8-39e351133744" />

在word里面有
## 如何运行?
环境要求:
Qt 6.9+ [下载链接](https://www.qt.io/download)
CMake 3.20+
## 编译步骤
bash
### 克隆仓库
git clone https://github.com/你的用户名/Compiler-Frontend.git

### 构建项目
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="你的Qt安装路径" ..  #示例：/home/user/Qt/6.9.1/gcc_64
make

### 运行程序
./CompilerFrontend
[简易编译器前端实现readme.docx](https://github.com/user-attachments/files/21254809/readme.docx)

