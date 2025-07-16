// lexer.cpp
#include "lexer.h"
//#include <cctype>
#include <iostream>
#include <unordered_map>
#include <QDebug>
Lexer::Lexer(const std::string& source) : source(source),position(0),line(1),column(1) {
    //tokens = scanTokens();//初始化时扫描token
    startLine = 1;
    startColumn = 1;
    source_length = source.length();
}

/*void Lexer::scanAllTokens() {

    while (!isAtEnd()) {
        start = position;
        Token token = scanToken();  // 获取单个Token
        tokens.push_back(token);    // 存入tokens缓存
    }
}*/

bool Lexer::isAtEnd() const {
    return position >= source_length;
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char c = source[position];
    position++;
    // 更新行列位置
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }


    //qDebug() << "Advance: '" << c << "' (line: " << line << ", column: " << column << ")\n";
    return c;
}

void Lexer::ratreat() {
    if (position == 0) return;
    position--;
    char c = source[position];
    if (c == '\n') {
        line--;
        // 需重新计算当前行长度才能准确恢复 column，简化处理可直接置 0
        column = 1;
    } else {
        column--;
    }
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[position];
}

char Lexer::peekNext() const {
    if (position + 1 >= source.length()) return '\0';
    return source[position + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[position] != expected) return false;

    advance();
    return true;
}

// 跳过空白字符和注释
void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
        case ' ':
        case '\t':
        case '\r':
            advance();
            break;
        case '\n':
            //line++;
            //column = 1;
            advance();
            break;
        case '/':
            if (peekNext() == '/') {
                // 单行注释
                while (peek() != '\n' &&peek()!='\r' && !isAtEnd()) advance();
                // 新增：跳过注释后的换行符，由advance()统一处理行号
                if (peek() == '\r') {
                    advance(); // 跳过\r
                    if (peek() == '\n') {
                        advance(); // 跳过\n (处理\r\n情况)
                    }
                } else if (peek() == '\n') {
                    advance(); // 跳过\n
                }
            } else if (peekNext() == '*') {
                // 多行注释
                advance(); // 消费第一个/
                advance(); // 消费*

                while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
                    //if (peek() == '\n') line++;
                    advance();
                }

                if (!isAtEnd()) {
                    advance(); // 消费*
                    advance(); // 消费/
                }
            } else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

// 扫描所有Token
std::vector<Token> Lexer::scanTokens() {


    if (tokensGenerated) {
        // 已生成过，重置tokenIndex后返回现有tokens
        tokenIndex = 0;
        return this->tokens;
    }
    // 首次生成tokens
    std::vector<Token> tokens;
    tokens.clear();
    tokenIndex = 0;
    while (!isAtEnd()) {
        start = position;
        tokens.push_back(scanToken());
    }

    tokens.push_back({TokenType::EOF_TOKEN, "", line, column});
    tokensGenerated = true;  // 标记为已生成
    this->tokens =tokens;
    return tokens;
}

// 扫描单个Token
Token Lexer::scanToken() {
    //int currentColumn = column; // 保存当前列号
    skipWhitespace();
    start = position;
    startLine = line;
    startColumn = column;
    if (isAtEnd()) return {TokenType::EOF_TOKEN, "", startLine, startColumn};

    char c = advance();

    // 标识符或关键字
    if (isAlpha(c) || c == '_') return identifier();

    // 数字
    if (isDigit(c)) return number();

    // 单字符Token
    switch (c) {
    case '&':
        if (peek() == '&') {
            advance();
            return {TokenType::OPERATOR, "&&", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "&", startLine, startColumn};
        }
    case '|':
        if (peek() == '|') {
            advance();
            return {TokenType::OPERATOR, "||", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "|", startLine, startColumn};
        }

    case ';': return {TokenType::PUNCTUATOR, ";", startLine, startColumn};
    case '+':
        if (peek() == '+') {
            advance();
            return {TokenType::OPERATOR, "++", startLine, startColumn};
        } else if (peek() == '=') {
            advance();
            return {TokenType::OPERATOR, "+=", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "+", startLine, startColumn};
        }
    case '-':
        if (peek() == '-') {
            advance();
            return {TokenType::OPERATOR, "--", startLine, startColumn};
        } else if (peek() == '=') {
            advance();
            return {TokenType::OPERATOR, "-=", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "-", startLine, startColumn};
        }
        //case '+': return {TokenType::OPERATOR, "+", startLine, startColumn};
    //case '-': return {TokenType::OPERATOR, "-", startLine, startColumn};
    case '*':
        if (peek() == '=') {
            advance();
            return {TokenType::OPERATOR, "*=", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "*", startLine, startColumn};
        }
    case '/':
        if (peek() == '=') {
            advance();
            return {TokenType::OPERATOR, "/=", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "/", startLine, startColumn};
        }
    //case '*': return {TokenType::OPERATOR, "*", startLine, startColumn};
    //case '/': return {TokenType::OPERATOR, "/", startLine, startColumn};
    //case '=': return {TokenType::OPERATOR, "=", startLine, startColumn};
    case ',': return {TokenType::PUNCTUATOR, ",", startLine, startColumn};
    case ':': return {TokenType::PUNCTUATOR, ":", startLine, startColumn};
    case '(': return {TokenType::PUNCTUATOR, "(", startLine, startColumn};
    case ')': return {TokenType::PUNCTUATOR, ")", startLine, startColumn};
    case '{': return {TokenType::PUNCTUATOR, "{", startLine, startColumn};
    case '}': return {TokenType::PUNCTUATOR, "}", startLine, startColumn};
    case '"': return string();
    //case '>': return {TokenType::OPERATOR, ">", startLine, startColumn};
    //case '<': return {TokenType::OPERATOR, "<", startLine, startColumn};
    case '=':
        if (peek() == '=') {  // 处理"=="
            advance();
            return {TokenType::OPERATOR, "==", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "=", startLine, startColumn};
        }
    case '<':
        if (peek() == '<') {
            advance();
            return {TokenType::OPERATOR, "<<", startLine, startColumn};
        }else if (peek() == '=') {  // 处理"<="
            advance();
            return {TokenType::OPERATOR, "<=", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "<", startLine, startColumn};
        }
    case '>':
        if (peek() == '>') {
            advance();
            return {TokenType::OPERATOR, ">>", startLine, startColumn};
        } else if (peek() == '=') {  // 处理">="
            advance();

            return {TokenType::OPERATOR, ">=", startLine, startColumn};
        } else {

            return {TokenType::OPERATOR, ">", startLine, startColumn};
        }
    case '!':
        if (peek() == '=') {  // 处理"!="
            advance();
            return {TokenType::OPERATOR, "!=", startLine, startColumn};
        } else {
            return {TokenType::OPERATOR, "!", startLine, startColumn};
        }


    //case '==': return {TokenType::OPERATOR, "==", startLine, startColumn};
    //case '>=': return {TokenType::OPERATOR, ">=", startLine, startColumn};
    //case '<=': return {TokenType::OPERATOR, "<=", startLine, startColumn};
    }

    // 错误处理
    std::cerr << "Unexpected character at startLine " << startLine << ", column " << startColumn << std::endl;
    return {TokenType::EOF_TOKEN, "", startLine, startColumn};
}

// 处理标识符和关键字
Token Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();

    std::string text = source.substr(start, position - start);

    // 检查是否为关键字

    if (keywords.find(text) != keywords.end()) {
        return {keywords[text], text, startLine, startColumn};
    }

    return {TokenType::IDENTIFIER, text, startLine, startColumn};
}

// 处理数字
Token Lexer::number() {
    while (isDigit(peek())) advance();

    // 处理小数部分
    if (peek() == '.' && isDigit(peekNext())) {
        advance(); // 消费.
        while (isDigit(peek())) advance();
    }

    std::string value = source.substr(start, position - start);
    return {TokenType::NUMBER, value, startLine, startColumn};
}

// 处理字符串
Token Lexer::string() {
    std::string value;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {line++;column=1;}
        if (peek() == '\\') {
            // 处理转义字符
            advance(); // 跳过'\'
            switch(peek()) {
            case '"': value += '"'; advance(); break; // 转义引号
            case '\\': value += '\\'; advance(); break; // 转义反斜杠
            case 'n': value += '\n'; advance(); break; // 换行符
            case 't': value += '\t'; advance(); break; // 制表符
            default: value += peek(); advance(); break; // 其他转义保留原样
            }
        } else {
            value += peek();
            advance();
        }
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string at line " << startLine << std::endl;
        return {TokenType::EOF_TOKEN, "", startLine, startColumn};
    }

    advance(); // 消费闭合引号

    return {TokenType::STRING, value, startLine, startColumn};
    // 提取字符串内容（去掉引号）
    //std::string value = source.substr(start + 1, position - start - 2);
    //return {TokenType::STRING, value, startLine, startColumn};
}

// 判断字符类型
bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

//接口
bool Lexer::hasNext() {
    return tokenIndex < tokens.size();
}

/*Token Lexer::nextToken() {
    if (hasNext()) {
        return tokens[tokenIndex++];
    }
    return {TokenType::EOF_TOKEN, "", 0, 0};
}
*/
