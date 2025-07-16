// lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include "token.h"
#include <unordered_map>
class Lexer {
private:
    std::string source;     // 源代码
    int position = 0;       // 当前字符位置
    int start = 0;          // 当前Token起始位置
    int line = 1;           // 当前行号
    int column = 1;         // 当前列号
    int startLine;   // Token起始行
    int startColumn; // Token起始列
    size_t source_length;

    // 辅助方法
    void ratreat();         // 回退一个字符
    char advance();         // 前进一个字符并返回当前字符
    char peek() const;      // 查看当前字符（不移动位置）
    char peekNext() const;  // 查看下一个字符（不移动位置）
    bool match(char expected); // 匹配并消费指定字符
    void skipWhitespace();  // 跳过空白字符和注释
    void skipComment();     // 跳过注释（单行和多行）
    bool tokensGenerated = false;  // 标记是否已生成tokens
    // Token识别
    Token scanToken();
    Token identifier();
    Token number();
    Token string();

    // 判断字符类型
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;

    //新增接口
    std::vector<Token> tokens;  // 缓存所有 Token
    int tokenIndex = 0;         // 当前 Token 索引
    //void scanAllTokens();       // 一次性扫描所有 Token（内部调用）

    // 定义关键字集合
    static std::unordered_map<std::string, TokenType> keywords;
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> scanTokens();

    bool isAtEnd() const;
    bool hasNext();        // 是否还有未处理的 Token
    bool hasNext() const { return tokenIndex < tokens.size(); }
    Token nextToken() {//获取下一个Token并推进索引
        return hasNext() ? tokens[tokenIndex++] : Token{TokenType::EOF_TOKEN, "", 0, 0};
    }
    Token peekNextToken() const {//查看下一个Token但不推进索引
        return (tokenIndex < tokens.size()) ? tokens[tokenIndex]
                                            : Token{TokenType::EOF_TOKEN, "", 0, 0};
    }

    Token peekAheadToken(int offset) const {
        if (offset < 0 || tokenIndex + offset >= tokens.size()) {
            return {TokenType::EOF_TOKEN, "", 0, 0};
        }
        return tokens[tokenIndex + offset];
    }

    void backupToken() {
        if (tokenIndex > 0) tokenIndex--;
    }
    void reset() {  // 重置方法
        tokenIndex = 0;
        //tokensGenerated = false;
    }
};
inline std::unordered_map<std::string, TokenType> Lexer::keywords = {
    // 控制流关键字
    {"if", TokenType::KEYWORD},
    {"else", TokenType::KEYWORD},
    {"switch", TokenType::KEYWORD},
    {"case", TokenType::KEYWORD},
    {"default", TokenType::KEYWORD},
    {"break", TokenType::KEYWORD},
    {"continue", TokenType::KEYWORD},
    {"goto", TokenType::KEYWORD},
    {"return", TokenType::KEYWORD},

    // 循环关键字
    {"for", TokenType::KEYWORD},
    {"while", TokenType::KEYWORD},
    {"do", TokenType::KEYWORD},

    // 数据类型关键字
    {"int", TokenType::KEYWORD},
    {"char", TokenType::KEYWORD},
    {"float", TokenType::KEYWORD},
    {"double", TokenType::KEYWORD},
    {"void", TokenType::KEYWORD},
    {"short", TokenType::KEYWORD},
    {"long", TokenType::KEYWORD},
    {"signed", TokenType::KEYWORD},
    {"unsigned", TokenType::KEYWORD},
    {"enum", TokenType::KEYWORD},
    {"struct", TokenType::KEYWORD},
    {"union", TokenType::KEYWORD},

    // 存储类关键字
    {"auto", TokenType::KEYWORD},
    {"static", TokenType::KEYWORD},
    {"register", TokenType::KEYWORD},
    {"extern", TokenType::KEYWORD},
    {"typedef", TokenType::KEYWORD},

    // 类型修饰符
    {"const", TokenType::KEYWORD},
    {"volatile", TokenType::KEYWORD},

    // 运算符关键字
    {"sizeof", TokenType::KEYWORD}
};


#endif // LEXER_H
