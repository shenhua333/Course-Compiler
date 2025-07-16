// token.h
#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    KEYWORD,       // 关键字
    IDENTIFIER,    // 标识符
    NUMBER,        // 数字
    OPERATOR,      // 运算符
    PUNCTUATOR,    // 标点符号
    STRING,        // 字符串
    EOF_TOKEN      // 文件结束
};

std::string tokenTypeToString(TokenType type);

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};

#endif // TOKEN_H
