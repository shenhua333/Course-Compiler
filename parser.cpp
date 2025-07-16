// parser.cpp
#include "parser.h"
#include "token.h"
#include "ast.h"
#include "error.h"
#include <iostream>
#include <stdexcept> // 用于抛出解析错误
#include <unordered_map>
#include <QDebug>
std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::KEYWORD:
        return "KEYWORD";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::OPERATOR:
        return "OPERATOR";
    case TokenType::PUNCTUATOR:
        return "PUNCTUATOR";
    case TokenType::STRING:
        return "STRING";
    case TokenType::EOF_TOKEN:
        return "EOF_TOKEN";
    // ... 其他 TokenType 值
    default:
        return "UNKNOWN";
    }
}

// 运算符优先级表（值越大优先级越高，与文法中的Expr1~Expr4对应）
const std::unordered_map<std::string, int> OP_PRECEDENCE = {
    {"**", 40},
    {"!",40},
    {"*", 30},
    {"/", 30},
    {"+", 20},
    {"-", 20},
    {"<<", 15},
    {">>", 15},
    {"<", 10},
    {"<=", 10},
    {">", 10},
    {">=", 10},
    {"==", 5},
    {"!=", 5},
    {"&&", 4},
    {"||", 3},
};

Parser::Parser(Lexer &lexer) : lexer(lexer)
{
    // nextToken();  // 初始化：读取第一个Token
    //  确保Lexer已准备好
    lexer.scanTokens(); // 确保Token已生成
    lexer.reset();      // 重置索引
    nextToken();
}

// 解析入口：解析整个程序
std::unique_ptr<Program> Parser::parse()
{
    return parseProgram();
}

// 推进到下一个Token
void Parser::nextToken()
{
    if (lexer.hasNext())
    {
        currentToken = lexer.nextToken();
        qDebug() << "推进到Token:" << QString::fromStdString(currentToken.value)
                 << "类型:" << static_cast<int>(currentToken.type);
    }
    else
    {
        currentToken = {TokenType::EOF_TOKEN, "", 0, 0}; // 结束标记
    }
}

// 匹配Token（类型+值）
bool Parser::match(TokenType type, const std::string &value)
{
    if (currentToken.type == type && currentToken.value == value)
    {
        nextToken(); // 匹配成功，推进到下一个Token
        return true;
    }
    return false;
}

// 匹配Token（仅类型）
bool Parser::match(TokenType type)
{
    if (currentToken.type == type)
    {
        nextToken();
        return true;
    }
    return false;
}

// 预期Token（类型+值），不匹配则报错
void Parser::expect(TokenType type, const std::string &value, const std::string &errorMsg)
{
    if (!match(type, value))
    {
        throw std::runtime_error(errorMsg + "（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + "）");
    }
}

// 预期Token（仅类型）
void Parser::expect(TokenType type, const std::string &errorMsg)
{
    if (!match(type))
    {
       throw std::runtime_error(errorMsg + "（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + "）");
    }
}

// 解析Program：多个函数定义
std::unique_ptr<Program> Parser::parseProgram()
{
    auto program = std::make_unique<Program>();
    qDebug() << "开始解析程序...";

    while (currentToken.type != TokenType::EOF_TOKEN)
    {
        // 添加详细日志
        qDebug() << "当前Token:" << QString::fromStdString(currentToken.value)
                 << "类型:" << static_cast<int>(currentToken.type);
        // 如果没有更多Token，跳出循环
        if (!lexer.hasNext()) {
            break;
        }
        // 尝试解析函数定义
        bool isFunctionDef = false;
        if (currentToken.type == TokenType::KEYWORD && typeKeywords.count(currentToken.value))
        {
            Token next1 = lexer.peekNextToken();
            Token next2 = lexer.peekAheadToken(1);
            if (next1.type == TokenType::IDENTIFIER &&
                next2.type == TokenType::PUNCTUATOR &&
                next2.value == "(")
            {
                isFunctionDef = true;
            }
        }

        if (isFunctionDef)
        {
            program->functions.push_back(parseFunctionDef());
        }
        else
        {
            // 解析全局语句
            qDebug() << "尝试解析全局语句";
            try
            {
                auto stmt = parseStmt();
                if (stmt)
                {
                    program->statements.push_back(std::move(stmt));
                }
            }
            catch (const std::exception &e)
            {
                qDebug() << "解析语句失败:" << e.what();
                if (currentToken.type != TokenType::EOF_TOKEN)
                { // 跳过错误Token
                    nextToken();
                }else break;
            }
        }
    }

    qDebug() << "解析完成，找到" << program->functions.size() << "个函数,"
             << program->statements.size() << "条全局语句";
    return program;
}

// 解析FunctionDef：返回类型 + 函数名 + 参数列表 + 函数体
std::unique_ptr<FunctionDef> Parser::parseFunctionDef()
{
    auto func = std::make_unique<FunctionDef>();

    // 解析返回类型（支持多种类型）
    if (currentToken.type != TokenType::KEYWORD || !typeKeywords.count(currentToken.value))
    {
        throw std::runtime_error(
            "函数定义需以有效类型开头（位置：行" + std::to_string(currentToken.line) +
            ", 列" + std::to_string(currentToken.column) + "）");
        //ErrorManager::instance().addError(ErrorType::SYNTAX_ERROR,currentToken.line,
        //    currentToken.column,"函数定义需以有效类型开头" );
    }
    func->returnType = currentToken.value; // 动态获取返回
    nextToken();
    // 解析函数名（标识符）
    std::string funcName = currentToken.value; // 保存当前Token值
    expect(TokenType::IDENTIFIER, "函数名应为标识符");
    func->name = funcName; // 注意：match后currentToken已更新，需用匹配前的值
    // 解析参数列表
    expect(TokenType::PUNCTUATOR, "(", "函数名后应跟'('");
    std::vector<Param> params;
    std::vector<std::pair<std::string, std::string>> paramList; // 参数名-类型对
    if (!match(TokenType::PUNCTUATOR, ")"))
    {                                    // 如果不是直接闭合的括号
        params = parseParamList(); // 解析参数列表
        // 将Param转换为符号表所需的格式
        for (const auto& param : params)
        {
            paramList.emplace_back(param.name, param.type);
        }
        expect(TokenType::PUNCTUATOR, ")", "参数列表后应跟')'");
    }
    func->params=params;
    for (const auto& param : params)
    {
        Symbol paramSym;
        paramSym.name = param.name;
        paramSym.type = param.type;
        paramSym.scope = symTable.getCurrentScope();
        paramSym.is_function = false;
        symTable.insert(paramSym);
        qDebug() << "已添加参数符号: " << param.name.c_str() << " (类型: " << param.type.c_str() << ")";
    }
    Symbol funcSym;
    funcSym.name = funcName;
    funcSym.type = func->returnType;
    funcSym.scope = symTable.getCurrentScope();
    funcSym.is_function = true;
    funcSym.params = paramList;
    symTable.insert(funcSym);
    // 解析函数体（Block）
    func->body = parseBlock();

    qDebug() << "解析函数: " << func->name
             << " (返回类型: " << func->returnType << ")\n";

    if (func->body)
    {
        qDebug() << "  函数体包含 " << func->body->statements.size() << " 条语句\n";
    }
    return func;
}

// 解析ParamList：参数列表（Param ("," Param)*）
std::vector<Param> Parser::parseParamList()
{
    std::vector<Param> params;

    // 解析第一个参数
    Param firstParam;
    if (currentToken.type != TokenType::KEYWORD || !typeKeywords.count(currentToken.value))
    {
        throw std::runtime_error(
            "参数类型应为有效类型（位置：行" + std::to_string(currentToken.line) +
            ", 列" + std::to_string(currentToken.column) + "）");
        //ErrorManager::instance().addError(ErrorType::TYPE_MISMATCH,currentToken.line,
                //currentToken.column,"参数类型应为有效类型:"+currentToken.value);
    }
    firstParam.type = currentToken.value;
    nextToken();

    std::string firstParamName = currentToken.value;
    expect(TokenType::IDENTIFIER, "参数名应为标识符");
    firstParam.name = firstParamName; // 同样需修正为匹配前的值
    params.push_back(firstParam);

    // 解析后续参数（"," Param）
    while (match(TokenType::PUNCTUATOR, ","))
    {
        Param param;
        // expect(TokenType::KEYWORD, "int", "参数类型应为'int'");
        if (currentToken.type != TokenType::KEYWORD || !typeKeywords.count(currentToken.value))
        {
            throw std::runtime_error(
                "参数类型应为有效类型（位置：行" + std::to_string(currentToken.line) +
                ", 列" + std::to_string(currentToken.column) + "）");
            //ErrorManager::instance().addError(ErrorType::TYPE_MISMATCH,currentToken.line,
                    //currentToken.column,"参数类型应为有效类型:"+currentToken.value);
        }
        param.type = currentToken.value;
        nextToken();

        std::string paramName = currentToken.value;
        expect(TokenType::IDENTIFIER, "参数名应为标识符");
        param.name = paramName; // 修正同上
        params.push_back(param);
    }

    return params;
}

// 解析Block：代码块（"{" Stmt* "}"）
std::unique_ptr<Block> Parser::parseBlock()
{
    auto block = std::make_unique<Block>();
    expect(TokenType::PUNCTUATOR, "{", "代码块应以'{'开头");
    //进入新作用域
    symTable.enterScope("block_" + std::to_string(symTable.getScopeCount()));
    // 解析语句列表（Stmt*）
    while (!match(TokenType::PUNCTUATOR, "}"))
    {                                             // 直到遇到"}"
        block->statements.push_back(parseStmt()); // 解析一条语句
    }
    // 退出当前作用域
    symTable.leaveScope();
    return block;
}

// 解析Stmt：根据当前Token判断语句类型
std::unique_ptr<Stmt> Parser::parseStmt()
{
    if (currentToken.type == TokenType::KEYWORD && typeKeywords.count(currentToken.value))
    {
        // 声明语句（int a = 10;）
        return parseDeclareStmt();
    }
    else if (currentToken.type == TokenType::IDENTIFIER)
    {
        // 可能是赋值语句（a = 20;）或表达式语句（printf(...);）
        // 先尝试匹配赋值（检查下一个Token是否为"="）
        Token next = lexer.peekNextToken(); // 假设Lexer有peek方法，查看下一个Token不推进
        if (next.type == TokenType::OPERATOR && next.value == "=")
        {
            return parseAssignStmt();
        }
        else
        {
            return parseExprStmt(); // 表达式语句
        }
    }
    else if (currentToken.type == TokenType::KEYWORD && currentToken.value == "if")
    {
        // if语句
        return parseIfStmt();
    }
    else if (currentToken.type == TokenType::KEYWORD && currentToken.value == "while") {
        return parseWhileStmt();
    }
    else if (currentToken.type == TokenType::KEYWORD && currentToken.value == "for") {
        return parseForStmt();
    }
    else if (currentToken.type == TokenType::KEYWORD && currentToken.value == "return")
    {
        // return语句
        return parseReturnStmt();
    }
    else
    {
        throw std::runtime_error(
            "未知语句类型（位置：行" + std::to_string(currentToken.line) +
            ", 列" + std::to_string(currentToken.column) + "）");
        //ErrorManager::instance().addError(ErrorType::SYNTAX_ERROR,currentToken.line,
                                          //currentToken.column,"未知语句类型:"+currentToken.value);
    }
}

// 解析DeclareStmt：声明语句（int a = 10;）
std::unique_ptr<DeclareStmt> Parser::parseDeclareStmt(bool consumeSemicolon)
{
    auto stmt = std::make_unique<DeclareStmt>();

    // 获取类型关键字（动态支持所有数据类型关键字）
    stmt->type = currentToken.value;
    nextToken();

    // 变量名（标识符）
    std::string varName = currentToken.value;
    expect(TokenType::IDENTIFIER, "声明语句中变量名应为标识符");
    stmt->varName = varName; // 修正为匹配前的值
    // 添加到符号表（记录变量）
    Symbol varSym;
    varSym.name = varName;
    varSym.type = stmt->type;
    varSym.scope = symTable.getCurrentScope();
    varSym.is_function = false;

    // 可选的初始化值（"=" Expr）
    if (match(TokenType::OPERATOR, "="))
    {
        stmt->initValue = parseExpr(); // 解析初始化表达式
        varSym.is_initialized = true;
    }else  varSym.is_initialized = false;
    symTable.insert(varSym);
    // nextToken();  // 跳过标识符
    //  语句结束符";"
    if (consumeSemicolon)
    {
        expect(TokenType::PUNCTUATOR, ";", "声明语句应以';'结束");
    }
    //expect(TokenType::PUNCTUATOR, ";", "声明语句应以';'结束");
    // program->statements.push_back(stmt);
    return stmt;
}

// 获取表达式的数据类型
// 参数: expr - 表达式AST节点
// 返回: 类型字符串（如"int"、"char"等）
std::string Parser::getExprType(Expr *expr)
{
    if (!expr)
    {
        throw std::runtime_error("空表达式节点（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + ")");
        //ErrorManager::instance().addError(ErrorType::UNDEFINED_VARIABLE,currentToken.line,
                                          //currentToken.column,"空表达式节点");
    }

    // 处理基础表达式（数字、标识符、函数调用等）
    if (auto *primaryExpr = dynamic_cast<PrimaryExpr *>(expr))
    {
        switch (primaryExpr->type)
        {
        case PrimaryExpr::IDENTIFIER:
            // 变量引用：从符号表查询类型
            {
                Symbol *sym = symTable.lookup(primaryExpr->identifier);
                if (!sym)
                {
                    throw std::runtime_error("未声明的标识符: " + primaryExpr->identifier +
                                             "（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + ")");
                    //ErrorManager::instance().addError(ErrorType::UNDEFINED_VARIABLE,currentToken.line,
                                                      //currentToken.column,"未声明的标识符:"+primaryExpr->identifier);
                }
                return sym->type;
            }
        case PrimaryExpr::NUMBER:
            // 数字字面量默认为int类型
            return "int";
        case PrimaryExpr::CALL_EXPR:
            // 函数调用：从符号表查询函数返回类型
            {
                Symbol *sym = symTable.lookup(primaryExpr->callExpr->callee);
                if (!sym)
                {
                    throw std::runtime_error("未声明的函数: " + primaryExpr->callExpr->callee +
                                             "（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + ")");
                    //ErrorManager::instance().addError(ErrorType::UNDEFINED_VARIABLE,currentToken.line,
                                                      //currentToken.column,"未声明的函数: " + primaryExpr->callExpr->callee);
                }
                return sym->type;
            }
        default:
            throw std::runtime_error("不支持的基础表达式类型（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + ")");
            //ErrorManager::instance().addError(ErrorType::INVALID_OPERATION,currentToken.line,
            //                                  currentToken.column,"不支持的基础表达式类型");
        }
    }
    // 处理二元表达式（如a + b）
    else if (auto *binaryExpr = dynamic_cast<BinaryExpr *>(expr))
    {
        // 二元表达式：取左操作数类型（简化处理）
        return getExprType(binaryExpr->left.get());
    }

    // 其他表达式类型可在此扩展
    //ErrorManager::instance().addError(ErrorType::INVALID_OPERATION,currentToken.line,
    //                                  currentToken.column,"不支持的基础表达式类型");
    throw std::runtime_error("不支持的表达式类型（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + ")");

}

bool Parser::isTypeCompatible(const std::string &targetType, const std::string &sourceType)
{
    // 基本类型匹配
    if (targetType == sourceType)
        return true;

    // 允许隐式转换（如int→float）
    if (targetType == "float" && sourceType == "int")
        return true;

    // 指针类型检查（简化）
    if (targetType == "char*" && sourceType == "char*")
        return true;

    return false;
}

// 解析AssignStmt：赋值语句（a = 20;）
std::unique_ptr<AssignStmt> Parser::parseAssignStmt()
{
    auto stmt = std::make_unique<AssignStmt>();

    // 变量名（标识符）
    stmt->varName = currentToken.value; // 保存当前标识符
    // 从符号表获取变量类型
    Symbol *symbol = symTable.lookup(stmt->varName);
    if (!symbol)
    {
        throw std::runtime_error("未定义的变量: " + stmt->varName +"（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + "）");
    }
    stmt->varType = symbol->type; // 记录变量类型
    nextToken();                  // 跳过标识符

    // 匹配"="
    expect(TokenType::OPERATOR, "=", "赋值语句中应有'='");
    // 赋值表达式
    stmt->value = parseExpr();
    stmt->exprType = getExprType(stmt->value.get()); // 获取表达式类型

    // 类型检查
    if (!isTypeCompatible(stmt->varType, stmt->exprType))
    {
        throw std::runtime_error("类型不匹配: 无法将 " + stmt->exprType + " 赋值给 " + stmt->varType+"（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + "）");
    }

    // 语句结束符";"
    expect(TokenType::PUNCTUATOR, ";", "赋值语句应以';'结束");

    return stmt;
}

// 解析IfStmt：if语句（if (a > 5) { ... } else { ... }）
std::unique_ptr<IfStmt> Parser::parseIfStmt()
{
    auto stmt = std::make_unique<IfStmt>();

    // 跳过"if"
    nextToken();

    // 条件表达式（"(" Expr ")"）
    expect(TokenType::PUNCTUATOR, "(", "if后应跟'('");
    stmt->condition = parseExpr(); // 解析条件
    expect(TokenType::PUNCTUATOR, ")", "条件表达式后应跟')'");

    // then分支语句：检查是否有大括号
    if (currentToken.type == TokenType::PUNCTUATOR && currentToken.value == "{")
    {

        auto block = parseBlock();
        stmt->thenStmt = std::make_unique<CompoundStmt>(std::move(block));
    }
    else
    {
        auto singleStmt = parseStmt();
        auto block = std::make_unique<Block>();
        block->statements.push_back(std::move(singleStmt));
        stmt->thenStmt = std::make_unique<CompoundStmt>(std::move(block));
    }
    // 可选的else分支
    if (match(TokenType::KEYWORD, "else"))
    {
        // else分支语句：检查是否有大括号
        if (currentToken.type == TokenType::PUNCTUATOR && currentToken.value == "{")
        {
            auto block = parseBlock();
            stmt->elseStmt = std::make_unique<CompoundStmt>(std::move(block));
        }
        else
        {
            stmt->elseStmt = parseStmt(); // 解析单个语句
        }
    }
    return stmt;
}
// 解析WhileStmt：while语句（while (condition) { ... }）
std::unique_ptr<Stmt> Parser::parseWhileStmt() {
    //auto stmt = std::make_unique<WhileStmt>();
    int line = currentToken.line;
    int column = currentToken.column;
    // 跳过"while"
    nextToken();

    // 解析条件表达式
    expect(TokenType::PUNCTUATOR, "(", "while后应跟'('");
    auto condition = parseExpr();
    expect(TokenType::PUNCTUATOR, ")", "条件表达式后应跟')'");

    // 解析循环体（支持块语句和单语句）
    std::unique_ptr<Stmt> body;
    if (currentToken.type == TokenType::PUNCTUATOR && currentToken.value == "{") {
        auto block = parseBlock();
        body = std::make_unique<CompoundStmt>(std::move(block));
    } else {
        body = parseStmt();
    }

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), line, column);
}

// 解析ForStmt：for语句（for (init; condition; increment) { ... }）
std::unique_ptr<Stmt> Parser::parseForStmt() {
    //auto stmt = std::make_unique<ForStmt>();
    int line = currentToken.line;
    int column = currentToken.column;
    // 跳过"for"
    nextToken();
    expect(TokenType::PUNCTUATOR, "(", "for后应跟'('");

    // 解析初始化语句（可选）
    std::unique_ptr<Stmt> init;
    if (currentToken.type != TokenType::PUNCTUATOR || currentToken.value != ";") {
        // 支持声明语句或表达式语句
        if (currentToken.type == TokenType::KEYWORD && typeKeywords.count(currentToken.value)) {
            init = parseDeclareStmt(false);
        } else if (currentToken.type == TokenType::IDENTIFIER &&
                   lexer.peekNextToken().type == TokenType::OPERATOR &&
                   lexer.peekNextToken().value == "=") {
            init = parseAssignStmt();
        } else {
            init = parseExprStmt();
        }
    }
    expect(TokenType::PUNCTUATOR, ";", "初始化语句后应跟';'");

    // 解析条件表达式（可选）
    std::unique_ptr<Expr> condition;
    if (currentToken.type != TokenType::PUNCTUATOR || currentToken.value != ";") {
        if (currentToken.type == TokenType::EOF_TOKEN || currentToken.value.empty()) {
            throw std::runtime_error("条件表达式为空（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + ")");
        }condition = parseExpr();
        // 显式检查并消费条件表达式后的分号
        expect(TokenType::PUNCTUATOR, ";", "条件表达式后应跟';'");
    } else {
        // 没有条件表达式时跳过分号
        nextToken();
    }

    // 解析增量表达式（可选）
    std::unique_ptr<Expr> increment;
    if (currentToken.type != TokenType::PUNCTUATOR || currentToken.value != ")") {
        increment = parseExpr();
    }
    expect(TokenType::PUNCTUATOR, ")", "for循环增量后应跟')'");

    // 解析循环体（支持块语句和单语句）
    std::unique_ptr<Stmt> body;
    if (currentToken.type == TokenType::PUNCTUATOR && currentToken.value == "{") {
        auto block = parseBlock();
        body = std::make_unique<CompoundStmt>(std::move(block));
    } else {
        body = parseStmt();
    }

    return std::make_unique<ForStmt>(std::move(init), std::move(condition),
                                     std::move(increment), std::move(body), line, column);
}

// 解析ReturnStmt：return语句（return 0;）
std::unique_ptr<ReturnStmt> Parser::parseReturnStmt()
{
    auto stmt = std::make_unique<ReturnStmt>();

    // 跳过"return"
    nextToken();

    // 可选的返回值表达式
    if (currentToken.type != TokenType::PUNCTUATOR || currentToken.value != ";")
    {
        stmt->returnValue = parseExpr(); // 解析返回值
    }

    // 语句结束符";"
    expect(TokenType::PUNCTUATOR, ";", "return语句应以';'结束");

    return stmt;
}

// 解析ExprStmt：表达式语句（printf("hello");）
std::unique_ptr<ExprStmt> Parser::parseExprStmt()
{
    auto stmt = std::make_unique<ExprStmt>();

    // 解析表达式
    stmt->expr = parseExpr();

    // 语句结束符";"
    expect(TokenType::PUNCTUATOR, ";", "表达式语句应以';'结束");

    return stmt;
}

// 解析Expr：表达式（入口，调用二元表达式解析）
std::unique_ptr<Expr> Parser::parseExpr()
{
    return parseBinaryExpr(0); // 从最低优先级开始解析
}

// 解析BinaryExpr：二元表达式（处理运算符优先级）
std::unique_ptr<Expr> Parser::parseBinaryExpr(int minPrecedence)
{
    // 先解析左侧基础表达式
    // auto left = parsePrimaryExpr();
    std::unique_ptr<Expr> left = parsePrimaryExpr(); // 基类指针接收子类对象
    // 循环处理右侧运算符和表达式（优先级攀爬法）
    while (true)
    {
        // 当前Token是否为运算符
        if (currentToken.type != TokenType::OPERATOR)
        {
            break; // 不是运算符，结束循环
        }

        // 获取当前运算符的优先级
        std::string op = currentToken.value;
        auto it = OP_PRECEDENCE.find(op);
        if (it == OP_PRECEDENCE.end())
        {
            break; // 未知运算符，结束循环
        }
        int precedence = it->second;

        // 如果当前运算符优先级低于最小要求，结束循环
        if (precedence < minPrecedence)
        {
            break;
        }

        // 匹配并消耗当前运算符
        nextToken();

        // 解析右侧表达式（优先级要求更高，避免改变运算顺序）
        auto right = parseBinaryExpr(precedence + 1);

        // 构建二元表达式节点，合并左右表达式
        auto binary = std::make_unique<BinaryExpr>();
        if (op == "+="||op == "-=") {
            // 创建 a = a + b 的形式
            auto assign = std::make_unique<BinaryExpr>();
            assign->op = (op == "+=")?"+":"-";
            assign->left = std::move(left);
            assign->right = std::move(right);
            //auto assign = std::make_unique<BinaryExpr>();
            binary->op = "=";
            binary->left=std::move(left);
            binary->right = std::move(right);
            left = std::move(binary);
        } else if (op == "*="||op =="/=") {
            auto assign = std::make_unique<BinaryExpr>();
            assign->op = (op == "*=")?"*":"/";
            assign->left = std::move(left);
            assign->right = std::move(right);

            binary->op = "=";
            binary->left=std::move(left);
            binary->right = std::move(right);
            left = std::move(binary);
        } else {
            binary->op = op;
            binary->left = std::move(left);
            binary->right = std::move(right);
            left = std::move(binary);
        }
    }

    return left;
}

// 解析PrimaryExpr：基础表达式（数字、标识符、字符串、括号表达式）
std::unique_ptr<PrimaryExpr> Parser::parsePrimaryExpr()
{
    auto expr = std::make_unique<PrimaryExpr>();

    if (currentToken.type == TokenType::NUMBER)
    {
        // 数字
        expr->type = PrimaryExpr::NUMBER;
        expr->numberValue = currentToken.value; // 保存数字值（匹配前的值）
        nextToken();
    }
    else if (currentToken.type == TokenType::IDENTIFIER)
    {
        // 标识符
        std::string ident = currentToken.value;
        nextToken();

        // 检查是否已声明
        if (!symTable.lookup(ident))
        {
            throw std::runtime_error("未声明的标识符：" + ident+"（位置：行" + std::to_string(currentToken.line) + ", 列" + std::to_string(currentToken.column) + "）");
        }

        // 检查是否为函数调用（标识符后紧跟'('）
        if (match(TokenType::PUNCTUATOR, "("))
        {
            expr->type = PrimaryExpr::CALL_EXPR;
            expr->callExpr = std::make_unique<CallExpr>();
            expr->callExpr->callee = ident; // 函数名

            // 解析参数列表
            std::vector<std::unique_ptr<Expr>> args;
            if (!match(TokenType::PUNCTUATOR, ")"))
            {
                // 解析第一个参数
                args.push_back(parseExpr());
                // 解析后续参数（逗号分隔）
                while (match(TokenType::PUNCTUATOR, ","))
                {
                    args.push_back(parseExpr());
                }
                // 转移参数所有权到callExpr
                for (auto &arg : args)
                {
                    expr->callExpr->arguments.push_back(arg.release());
                }
                qDebug() << "尝试匹配右括号，当前Token: " << QString::fromStdString(currentToken.value)
                         << "(类型: " << static_cast<int>(currentToken.type) << ")";
                expect(TokenType::PUNCTUATOR, ")", "函数调用缺少闭合')'");

            }
        }
        else if (match(TokenType::OPERATOR, "++")) {
            auto unaryExpr = std::make_unique<UnaryExpr>();
            unaryExpr->op = "++";
            unaryExpr->isPostfix = true;
            auto primary = std::make_unique<PrimaryExpr>();
            primary->type = PrimaryExpr::IDENTIFIER;
            primary->identifier = ident;
            unaryExpr->expr = std::move(primary);
            expr->type = PrimaryExpr::UNARY_EXPR;
            expr->unaryExpr = std::move(unaryExpr);
        }
        else if (match(TokenType::OPERATOR, "--")) {
            auto unaryExpr = std::make_unique<UnaryExpr>();
            unaryExpr->op = "--";
            unaryExpr->isPostfix = true;
            auto primary = std::make_unique<PrimaryExpr>();
            primary->type = PrimaryExpr::IDENTIFIER;
            primary->identifier = ident;
            unaryExpr->expr = std::move(primary);
            expr->type = PrimaryExpr::UNARY_EXPR;
            expr->unaryExpr = std::move(unaryExpr);
        }
        else
        {
            // 普通标识符
            expr->type = PrimaryExpr::IDENTIFIER;
            expr->identifier = ident;
        }
        /*expr->type = PrimaryExpr::IDENTIFIER;
        expr->identifier = currentToken.value;  // 保存标识符
        nextToken();*/
    }
    else if (currentToken.type == TokenType::STRING)
    {
        // 字符串字面量（如"hello"）
        expr->type = PrimaryExpr::STRING;
        expr->stringValue = currentToken.value; // 保存字符串值
        nextToken();
    }
    else if (match(TokenType::PUNCTUATOR, "("))
    {
        // 括号表达式（(Expr)）
        expr->type = PrimaryExpr::PAREN_EXPR;
        expr->parenExpr = parseExpr(); // 解析括号内的表达式
        expect(TokenType::PUNCTUATOR, ")", "括号表达式缺少闭合')'");
    }
    else
    {
        throw std::runtime_error(
            "不支持的基础表达式（位置：行" + std::to_string(currentToken.line) +
            ", 列" + std::to_string(currentToken.column) + "）");
    }

    return expr;
}
