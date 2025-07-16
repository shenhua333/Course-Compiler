// ast.h
#ifndef AST_H
#define AST_H

#include <vector>
#include <memory>
#include <string>
class Program;
class FunctionDef;
class Param;
class Block;
class Stmt;
class DeclareStmt;
class AssignStmt;
class CompoundStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class BreakStmt;
class ReturnStmt;
class ExprStmt;
class Expr;
class BinaryExpr;
class PrimaryExpr;
struct CallExpr;
class UnaryExpr;
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    // 为每个AST节点类型声明访问方法
    virtual void visit(Program& node) = 0;
    virtual void visit(FunctionDef& node) = 0;
    virtual void visit(Block& node) = 0;
    virtual void visit(DeclareStmt& node) = 0;
    virtual void visit(AssignStmt& node) = 0;
    virtual void visit(CompoundStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(ForStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(BreakStmt& node) = 0;
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(PrimaryExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
};
// AST节点基类（所有节点的共同接口）
class ASTNode {
public:
    virtual ~ASTNode() = default;  // 虚析构函数，确保子类正确析构
    std::string returnType;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// 语句基类（所有语句的父类）
class Stmt : public ASTNode {
public:
    void accept(ASTVisitor& visitor) override = 0; // 添加纯虚accept方法
};

// 程序节点（整个程序）
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    std::vector<std::unique_ptr<class FunctionDef>> functions;  // 函数列表
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// 函数定义节点
class FunctionDef : public ASTNode {
public:
    std::string returnType;  // 返回类型（如"int"）
    std::string name;        // 函数名（如"main"）
    std::vector<class Param> params;  // 参数列表
    std::unique_ptr<class Block> body;  // 函数体（代码块）
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// 参数节点（函数参数）
class Param {
public:
    std::string type;  // 参数类型（如"int"）
    std::string name;  // 参数名（如"a"）
};

// 代码块节点（{}包裹的语句）
class Block : public ASTNode {
public:
    std::vector<std::unique_ptr<class Stmt>> statements;  // 语句列表
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};



// 声明语句节点（如int a = 10;）
class DeclareStmt : public Stmt {
public:
    std::string type;  // 类型（如"int"）
    std::string varName;  // 变量名（如"a"）
    std::unique_ptr<class Expr> initValue;  // 初始化值（可选，如10）
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// 赋值语句节点（如a = 20;）
class AssignStmt : public Stmt {
public:
    std::string varName;  // 变量名（如"a"）
    std::unique_ptr<Expr> value;  // 赋值表达式（如20）
    std::string varType;    // 变量类型（从符号表获取）
    std::string exprType;   // 表达式类型（推导得出）
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// 复合语句节点（用于包装代码块）
class CompoundStmt : public Stmt {
public:
    std::unique_ptr<Block> body;  // 存储代码块

    // 构造函数
    explicit CompoundStmt(std::unique_ptr<Block> block)
        : body(std::move(block)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// if语句节点
class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;  // 条件表达式（如a > 5）
    std::unique_ptr<Stmt> thenStmt;   // if分支语句
    std::unique_ptr<Stmt> elseStmt;   // else分支语句（可选）
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// while语句节点（继承自Stmt，与IfStmt同级）
class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;  // 循环条件（如i < 10）
    std::unique_ptr<Stmt> body;       // 循环体（如代码块或单条语句）
    int line;  // 位置信息（行号，用于错误提示）
    int column;

    // 构造函数（初始化位置信息）
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b, int l, int c)
        : condition(std::move(cond)), body(std::move(b)), line(l), column(c) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// for语句节点（继承自Stmt）
class ForStmt : public Stmt {
public:
    std::unique_ptr<Stmt> init;       // 初始化语句（如int i=0）
    std::unique_ptr<Expr> condition;  // 循环条件（如i < 10）
    std::unique_ptr<Expr> increment;  // 增量表达式（如i++）
    std::unique_ptr<Stmt> body;       // 循环体
    int line;
    int column;

    // 构造函数
    ForStmt(std::unique_ptr<Stmt> i, std::unique_ptr<Expr> cond,
            std::unique_ptr<Expr> inc, std::unique_ptr<Stmt> b, int l, int c)
        : init(std::move(i)), condition(std::move(cond)),
        increment(std::move(inc)), body(std::move(b)), line(l), column(c) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};
// return语句节点
class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> returnValue;  // 返回值（可选，如0）
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class BreakStmt : public Stmt {
public:
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// 表达式语句节点（如printf("hello");）
class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;  // 表达式
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// 表达式基类（所有表达式的父类）
class Expr : public ASTNode {

};

// 二元表达式节点（如a + b）
class BinaryExpr : public Expr {
public:
    std::string op;  // 运算符（如"+"、"*"、">"）
    std::unique_ptr<Expr> left;  // 左操作数
    std::unique_ptr<Expr> right; // 右操作数
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

struct CallExpr {
    std::string callee;               // 被调用函数名
    std::vector<Expr*> arguments;          // 参数表达式列表
    int line;                         // 行号信息
    int column;                       // 列号信息
};

class UnaryExpr : public Expr {
public:
    std::string op;                  // 运算符（如"++"、"--"、"!"）
    std::unique_ptr<Expr> expr;      // 操作数表达式
    bool isPostfix;                  // true表示后缀运算符（如i++），false表示前缀（如++i）

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }
};

// 基础表达式节点（数字、标识符、字符串、括号表达式）
class PrimaryExpr : public Expr {
public:
    enum Type { NUMBER, IDENTIFIER, STRING, PAREN_EXPR ,CALL_EXPR,UNARY_EXPR };
    Type type;  // 基础表达式类型

    // 根据类型存储对应值
    std::string numberValue;    // 数字值（如"123"）
    std::string identifier;     // 标识符（如"a"、"printf"）
    std::string stringValue;    // 字符串值（如"hello"）
    std::unique_ptr<Expr> parenExpr;  // 括号表达式（如(a + b)）
    std::unique_ptr<CallExpr> callExpr;//函数调用
    std::unique_ptr<UnaryExpr> unaryExpr;//一元

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

#endif // AST_H
