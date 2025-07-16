// parser.h
#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <string>
#include "lexer.h"  // 依赖词法分析器的Token
#include "ast.h"    // 依赖AST节点
#include <unordered_set>
#include "symbol.h"
class Parser {
private:
    Lexer& lexer;  // 词法分析器（提供Token流）
    Token currentToken;  // 当前读取的Token
    SymbolTable symTable; // 新增符号表
    const std::unordered_set<std::string> typeKeywords={"int","char","float","double","void",
        "short",
        "long",
        "signed",
        "unsigned",
        "enum",
        "struct",
        "union",
};
    // 辅助函数：推进到下一个Token
    void nextToken();

    // 辅助函数：匹配预期的Token类型和值，不匹配则报错
    bool match(TokenType type, const std::string& value);
    // 重载：仅匹配类型（适用于无需检查值的情况）
    bool match(TokenType type);

    // 辅助函数：预期某个Token，不匹配则抛出异常（含错误位置）
    void expect(TokenType type, const std::string& value, const std::string& errorMsg);
    void expect(TokenType type, const std::string& errorMsg);

    // 获取表达式的类型
    std::string getExprType(Expr* expr);
    bool isTypeCompatible(const std::string& targetType, const std::string& sourceType);
    // 解析函数：对应文法规则（核心）
    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<FunctionDef> parseFunctionDef();
    std::vector<Param> parseParamList();
    std::unique_ptr<Block> parseBlock();
    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<DeclareStmt> parseDeclareStmt(bool consumeSemicolon = true);
    std::unique_ptr<AssignStmt> parseAssignStmt();
    std::unique_ptr<IfStmt> parseIfStmt();
    std::unique_ptr<ReturnStmt> parseReturnStmt();
    std::unique_ptr<ExprStmt> parseExprStmt();
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseBinaryExpr(int minPrecedence);  // 处理运算符优先级
    std::unique_ptr<PrimaryExpr> parsePrimaryExpr();
    std::unique_ptr<Stmt> parseWhileStmt(); // 添加while循环解析函数声明
    std::unique_ptr<Stmt> parseForStmt();
    //std::unique_ptr<WhileStmt> parseWhileStmt();
public:
    // 构造函数：接收词法分析器
    explicit Parser(Lexer& lexer);

    // 解析入口：返回整个程序的AST
    std::unique_ptr<Program> parse();
};

#endif // PARSER_H
