// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include "token.h"
#include "parser.h"
#include "ast.h"
#include "error.h"
#include <QTreeWidgetItem>
#include "CodeHighlighter.h"
#include <QDebug>
#include <regex>
void addASTNodeToTree(ASTNode* node, QTreeWidgetItem* parentItem);
void addProgramNode(Program* program, QTreeWidgetItem* parent);
void addFunctionDefNode(FunctionDef* func, QTreeWidgetItem* parent);
void addBlockNode(Block* block, QTreeWidgetItem* parent);
void addStmtNode(Stmt* stmt, QTreeWidgetItem* parent);
void addDeclareStmtNode(DeclareStmt* declare, QTreeWidgetItem* parent);
void addAssignStmtNode(AssignStmt* assign, QTreeWidgetItem* parent);
void addIfStmtNode(IfStmt* ifStmt, QTreeWidgetItem* parent);
void addForStmtNode(ForStmt* forStmt, QTreeWidgetItem* parent);

void addReturnStmtNode(ReturnStmt* returnStmt, QTreeWidgetItem* parent);
void addExprStmtNode(ExprStmt* exprStmt, QTreeWidgetItem* parent);
void addExprNode(Expr* expr, QTreeWidgetItem* parent);
void addBinaryExprNode(BinaryExpr* binary, QTreeWidgetItem* parent);
void addPrimaryExprNode(PrimaryExpr* primary, QTreeWidgetItem* parent);
void addCallExprNode(CallExpr* callExpr, QTreeWidgetItem* parent);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setWindowTitle("简易编译器前端");
    new CodeHighlighter(ui->codeEditor->document()); // 绑定代码高亮
    // 连接信号槽
    connect(ui->compileButton, &QPushButton::clicked, this, &MainWindow::compileButtonClicked);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::clearButtonClicked);
    connect(ui->errorList, &QListWidget::itemClicked, this, &MainWindow::errorListItemClicked);

    QWidget *centralWidget = this->centralWidget();
    if (centralWidget->layout()) {
        delete centralWidget->layout();
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    //  创建顶部按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(4);
    buttonLayout->setAlignment(Qt::AlignCenter); // 按钮居中显示

    // 添加按钮并设置样式
    ui->compileButton->setText("编译");
    ui->compileButton->setMinimumSize(80, 30);
    ui->pushButton->setText("清空");
    ui->pushButton->setMinimumSize(80, 30);

    buttonLayout->addWidget(ui->compileButton);
    buttonLayout->addWidget(ui->pushButton);

    // 添加到主布局
    mainLayout->addLayout(buttonLayout);

    // 创建中央内容区域的水平布局
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(3);

    //  创建左侧面板（代码编辑器和AST树）
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(4);
    leftLayout->setContentsMargins(5, 5, 3, 5);
    leftLayout->addWidget(ui->codeEditor);
    leftLayout->addWidget(ui->astTree);
    leftLayout->setStretch(0, 6); // 代码编辑器占60%高度
    leftLayout->setStretch(1, 4); // AST树占40%高度

    //  创建右侧面板（Token列表和错误列表）
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(4);
    rightLayout->setContentsMargins(3, 5, 5, 5);
    rightLayout->addWidget(ui->tokenList);
    rightLayout->addWidget(ui->errorList);
    rightLayout->setStretch(0, 6); // Token列表占60%高度
    rightLayout->setStretch(1, 4); // 错误列表占40%高度

    //  将面板添加到内容布局（7:3比例）
    contentLayout->addWidget(leftPanel, 7);
    contentLayout->addWidget(rightPanel, 3);

    //  将内容布局添加到主布局
    mainLayout->addLayout(contentLayout);
    mainLayout->setStretch(1, 1); // 内容区域占主要空间

    //  设置所有可伸缩部件的大小策略
    ui->codeEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->astTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tokenList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->errorList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 全局样式（浅色现代风）
    QString qss = R"(
        /* 基础字体 */
        * {
            font-family: "Segoe UI", sans-serif;
            font-size: 14px;
            color: #333;
            font-size: 13px;
        }

        /* 主窗口背景 */
        MainWindow {
            background-color: #f8f9fa;
        }

        /* 代码编辑器 */
        QTextEdit {
            background: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 10px;
            font-family: "Consolas", monospace;

        }
        //按钮
        QPushButton {
            background-color: #4CAF50;
            color: #FFFFFF;            /* 纯白，强制对比 */
            font-size: 16px;           /* 大字体，确保清晰 */
            padding: 6px 12px;        /* 增大按钮尺寸 */
            border: 2px solid red;     /* 调试用：显示按钮边界 */
        }


        /* Token列表 */
        QListWidget {
            border: none;
            background: white;
            padding: 5px;
        }
        QListWidget::item {
            padding: 4px 8px;
            border-radius: 4px;
        }
        QListWidget::item:hover {
            background: #f8f9fa;
        }

        /* AST树 */
        QTreeWidget {
            border: none;
            background: white;
            padding: 5px;
        }
        QTreeWidget::item {
            padding: 4px 8px;
        }
        QTreeWidget::item:selected {
            background: #e3f2fd;
            color: #1976d2;
        }
    )";
    setStyleSheet(qss);
}

//错误行高亮；
void MainWindow::highlightErrorLine(int line) {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor("#FFF59D")); // 黄色背景
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    QTextCursor cursor(ui->codeEditor->document()->findBlockByLineNumber(line - 1));
    cursor.movePosition(QTextCursor::EndOfLine);
    selection.cursor = cursor;
    ui->codeEditor->setExtraSelections({selection});
}
//清除代码编辑器内容、Token列表和AST树
void MainWindow::clearButtonClicked() {
    // 清除代码编辑器内容
    ui->codeEditor->clear();

    // 清空Token列表
    ui->tokenList->clear();

    // 清空AST树
    ui->astTree->clear();

    ui->errorList->clear();  // 清空错误列表
    errorLineMap.clear();
    // 清除可能的错误高亮
    ui->codeEditor->setExtraSelections({});
}
void MainWindow::showError(const QString &errorMsg, int lineNumber,int columnNumber = -1) {
    // 清除之前的错误高亮
    ui->codeEditor->setExtraSelections({});
    ui->errorList->clear();
    errorLineMap.clear();

    // 添加标题
    QListWidgetItem *titleItem = new QListWidgetItem("错误信息:");
    QFont font = titleItem->font();
    font.setBold(true);
    titleItem->setFont(font);
    titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsSelectable);
    ui->errorList->addItem(titleItem);

    // 添加错误信息
    QListWidgetItem *errorItem = new QListWidgetItem(errorMsg);
    ui->errorList->addItem(errorItem);

    // 如果有行号，保存映射关系并高亮显示
    if (lineNumber > 0) {
        errorLineMap[errorItem] = lineNumber;
        highlightErrorLine(lineNumber);
    }
}
void MainWindow::errorListItemClicked(QListWidgetItem *item)
{
    if (errorLineMap.contains(item))
    {
        int lineNumber = errorLineMap[item];
        highlightErrorLine(lineNumber);
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}


// 前向声明：递归构建AST树
void addASTNodeToTree(ASTNode* astNode, QTreeWidgetItem* parentItem);

void MainWindow::compileButtonClicked()
{

    // 清空之前的结果
    ui->tokenList->clear();
    ui->astTree->clear();
    ui->errorList->clear();
    errorLineMap.clear();
    ui->codeEditor->setExtraSelections({});
    // 获取代码
    QString code = ui->codeEditor->toPlainText();

    if (code.isEmpty()) {
        showError("错误: 请输入代码后再编译", -1);
        return;
    }

    // 词法分析
    Lexer lexer(code.toStdString());

    try {
        std::vector<Token> tokens = lexer.scanTokens();
        displayTokens(tokens);// 显示Token

        lexer.reset();
        // 下一步：语法分析（生成AST）
        Parser parser(lexer);
        std::unique_ptr<Program> program = parser.parse();  // 获取AST根节点

        //  构建AST树形结构
        if (program) {
            // 创建根节点
            QTreeWidgetItem* root = new QTreeWidgetItem(ui->astTree);
            root->setText(0, "ast树");

            // 直接处理Program节点
            //addProgramNode(program.get(), root);  // 使用修改后的addProgramNode
            QTreeWidgetItem* programItem = new QTreeWidgetItem(root);
            programItem->setText(0, "Program");

            // 处理全局语句
            if (!program->statements.empty()) {
                QTreeWidgetItem* stmtsItem = new QTreeWidgetItem(programItem);
                stmtsItem->setText(0, "Global Statements");
                for (auto& stmt : program->statements) {
                    addASTNodeToTree(stmt.get(), stmtsItem);
                }
            }

            // 处理函数
            if (!program->functions.empty()) {
                QTreeWidgetItem* funcsItem = new QTreeWidgetItem(programItem);
                funcsItem->setText(0, "Functions（函数）");
                for (auto& func : program->functions) {
                    addASTNodeToTree(func.get(), funcsItem);
                }
            }

            ui->astTree->expandAll();

            // 确保UI刷新
            ui->astTree->expandAll();
            QApplication::processEvents();
        }

    } catch (const std::exception& e) {
        // 捕获并显示错误
        std::string errorMsg = e.what();
        int lineNumber = -1;
        int columnNumber = -1;
        // 解析错误消息中的行号和列号 (格式: ... (line X, column Y))
        std::smatch match;
        if (std::regex_search(errorMsg, match, std::regex(R"(\（位置：行(\d+), 列(\d+)\）)"))) {
            lineNumber = std::stoi(match[1]);
            columnNumber = std::stoi(match[2]);
        }

        if (lineNumber == -1 || columnNumber == -1) {
            showError(QString::fromStdString(errorMsg), -1, -1);
        } else {
            showError(QString::fromStdString(errorMsg), lineNumber, columnNumber);
        }
    }
}

void MainWindow::displayTokens(const std::vector<Token>& tokens)
{
    for (const auto& token : tokens) {
        QString typeStr;

        switch (token.type) {
        case TokenType::KEYWORD: typeStr = "关键字"; break;
        case TokenType::IDENTIFIER: typeStr = "标识符"; break;
        case TokenType::NUMBER: typeStr = "数字"; break;
        case TokenType::OPERATOR: typeStr = "运算符"; break;
        case TokenType::PUNCTUATOR: typeStr = "标点"; break;
        case TokenType::STRING: typeStr = "字符串"; break;
        case TokenType::EOF_TOKEN: typeStr = "文件结束"; break;
        }

        QString itemText = QString("%1 [%2] (行:%3, 列:%4)")
                               .arg(QString::fromStdString(token.value))
                               .arg(typeStr)
                               .arg(token.line)
                               .arg(token.column);

        QListWidgetItem* item = new QListWidgetItem(itemText);

        // 根据Token类型设置不同颜色
        if (token.type == TokenType::KEYWORD) {
            item->setForeground(Qt::blue);
        } else if (token.type == TokenType::NUMBER) {
            item->setForeground(Qt::darkGreen);
        } else if (token.type == TokenType::STRING) {
            item->setForeground(Qt::darkMagenta);
        } else if (token.type == TokenType::OPERATOR) {
            item->setForeground(Qt::darkRed);
        }

        ui->tokenList->addItem(item);
    }
}

// 处理Program节点（根节点）
void addProgramNode(Program* program, QTreeWidgetItem* parent) {
    QTreeWidgetItem* programItem = new QTreeWidgetItem(parent);
    programItem->setText(0, "Program（程序）");  // 节点显示文本

    // 显示全局语句
    if (!program->statements.empty()) {
        QTreeWidgetItem* stmtsItem = new QTreeWidgetItem(programItem);
        //stmtsItem->setText(0, "全局语句（" + QString::number(program->statements.size()) + "条）");
        stmtsItem->setText(0, "Global Statements");
        for (auto& stmt : program->statements) {
            addASTNodeToTree(stmt.get(), stmtsItem);
            QApplication::processEvents();
        }
    }

    // 显示函数定义
    if (!program->functions.empty()) {
        QTreeWidgetItem* funcsItem = new QTreeWidgetItem(programItem);
        //funcsItem->setText(0, "函数定义（" + QString::number(program->functions.size()) + "个）");
        funcsItem->setText(0, "Functions");
        for (auto& func : program->functions) {
            addASTNodeToTree(func.get(), funcsItem);
            QApplication::processEvents();
        }
    }
}

// 处理FunctionDef节点（函数定义）
void addFunctionDefNode(FunctionDef* func, QTreeWidgetItem* parent) {
    QTreeWidgetItem* funcItem = new QTreeWidgetItem(parent);
    funcItem->setText(0,QString("Function: %1 (返回类型: %2)").arg(func->name.c_str()).arg(func->returnType.c_str()));
    //参数
    if (!func->params.empty()) {
        QTreeWidgetItem* paramsItem = new QTreeWidgetItem(funcItem);
        paramsItem->setText(0, "参数列表（" + QString::number(func->params.size()) + "个）");
        for (auto& param : func->params) { // 注意：param是值类型，直接使用
            QTreeWidgetItem* pItem = new QTreeWidgetItem(paramsItem);
            pItem->setText(0, QString("%1 %2").arg(param.type.c_str()).arg(param.name.c_str()));
        }
    }

    // 处理函数体（Block节点）
    if (func->body) {
        QTreeWidgetItem* bodyItem = new QTreeWidgetItem(funcItem);
        bodyItem->setText(0, "函数体：");
        addASTNodeToTree(func->body.get(), bodyItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* emptyBodyItem = new QTreeWidgetItem(funcItem);
        emptyBodyItem->setText(0, "函数体为空");
    }
}

// 处理Block节点（代码块）
void addBlockNode(Block* block, QTreeWidgetItem* parent) {
    QTreeWidgetItem* blockItem = new QTreeWidgetItem(parent);
    blockItem->setText(0, "Block（代码块）"+ QString::number(block->statements.size()) + "条语句");

    // 遍历代码块中的所有语句
    for (const auto& stmt : block->statements) {
        addASTNodeToTree(stmt.get(), blockItem);  // 递归处理语句节点
        QApplication::processEvents();
    }
}

// 处理 DeclareStmt 节点（声明语句，如 int a = 10;）
void addDeclareStmtNode(DeclareStmt* declare, QTreeWidgetItem* parent) {
    QTreeWidgetItem* declareItem = new QTreeWidgetItem(parent);
    declareItem->setText(0, QString("DeclareStmt: %1 %2")
                                .arg(declare->type.c_str())
                                .arg(declare->varName.c_str()));

    // 显示初始化值（如果有）
    if (declare->initValue) {
        QTreeWidgetItem* initItem = new QTreeWidgetItem(declareItem);
        initItem->setText(0, "初始化值：");
        addASTNodeToTree(declare->initValue.get(), initItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* noInitItem = new QTreeWidgetItem(declareItem);
        noInitItem->setText(0, "无初始化值");
    }
}


// 处理 AssignStmt 节点（赋值语句，如 a = 20;）
void addAssignStmtNode(AssignStmt* assign, QTreeWidgetItem* parent) {
    QTreeWidgetItem* assignItem = new QTreeWidgetItem(parent);
    assignItem->setText(0, QString("AssignStmt: %1 = ...").arg(assign->varName.c_str()));

    // 显示赋值表达式
    if (assign->value) {
        QTreeWidgetItem* valueItem = new QTreeWidgetItem(assignItem);
        valueItem->setText(0, "赋值表达式：");
        addASTNodeToTree(assign->value.get(), valueItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* invalidValueItem = new QTreeWidgetItem(assignItem);
        invalidValueItem->setText(0, "赋值表达式为空（无效）");
    }
}


// 处理 IfStmt 节点（if语句，如 if (a>5) { ... } else { ... }）
void addIfStmtNode(IfStmt* ifStmt, QTreeWidgetItem* parent) {
    QTreeWidgetItem* ifItem = new QTreeWidgetItem(parent);
    ifItem->setText(0, "IfStmt（条件语句）");

    // 显示条件表达式
    if (ifStmt->condition) {
        QTreeWidgetItem* condItem = new QTreeWidgetItem(ifItem);
        condItem->setText(0, "条件表达式：");
        addASTNodeToTree(ifStmt->condition.get(), condItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* invalidCondItem = new QTreeWidgetItem(ifItem);
        invalidCondItem->setText(0, "条件表达式为空（无效）");
    }

    // 显示then分支（if后的语句）
    if (ifStmt->thenStmt) {
        QTreeWidgetItem* thenItem = new QTreeWidgetItem(ifItem);
        thenItem->setText(0, "Then分支（条件为真时执行）：");
        addASTNodeToTree(ifStmt->thenStmt.get(), thenItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* emptyThenItem = new QTreeWidgetItem(ifItem);
        emptyThenItem->setText(0, "Then分支为空");
    }

    // 显示else分支（可选）
    if (ifStmt->elseStmt) {
        QTreeWidgetItem* elseItem = new QTreeWidgetItem(ifItem);
        elseItem->setText(0, "Else分支（条件为假时执行）：");
        addASTNodeToTree(ifStmt->elseStmt.get(), elseItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* noElseItem = new QTreeWidgetItem(ifItem);
        noElseItem->setText(0, "无Else分支");
    }
}
// 处理ForStmt节点（for循环语句）
void addForStmtNode(ForStmt* forStmt, QTreeWidgetItem* parent) {
    QTreeWidgetItem* forItem = new QTreeWidgetItem(parent);
    forItem->setText(0, "ForStmt（循环语句）");

    // 显示初始化语句
    if (forStmt->init) {
        QTreeWidgetItem* initItem = new QTreeWidgetItem(forItem);
        initItem->setText(0, "初始化语句：");
        addASTNodeToTree(forStmt->init.get(), initItem);
    } else {
        QTreeWidgetItem* noInitItem = new QTreeWidgetItem(forItem);
        noInitItem->setText(0, "无初始化语句");
    }

    // 显示循环条件
    if (forStmt->condition) {
        QTreeWidgetItem* condItem = new QTreeWidgetItem(forItem);
        condItem->setText(0, "循环条件：");
        addASTNodeToTree(forStmt->condition.get(), condItem);
    } else {
        QTreeWidgetItem* noCondItem = new QTreeWidgetItem(forItem);
        noCondItem->setText(0, "无条件循环");
    }

    // 显示增量表达式
    if (forStmt->increment) {
        QTreeWidgetItem* incItem = new QTreeWidgetItem(forItem);
        incItem->setText(0, "增量表达式：");
        addASTNodeToTree(forStmt->increment.get(), incItem);
    } else {
        QTreeWidgetItem* noIncItem = new QTreeWidgetItem(forItem);
        noIncItem->setText(0, "无增量表达式");
    }

    // 显示循环体
    if (forStmt->body) {
        QTreeWidgetItem* bodyItem = new QTreeWidgetItem(forItem);
        bodyItem->setText(0, "循环体：");
        addASTNodeToTree(forStmt->body.get(), bodyItem);
    } else {
        QTreeWidgetItem* emptyBodyItem = new QTreeWidgetItem(forItem);
        emptyBodyItem->setText(0, "循环体为空");
    }
}
// 处理WhileStmt节点（while循环语句）
void addWhileStmtNode(WhileStmt* whileStmt, QTreeWidgetItem* parent) {
    QTreeWidgetItem* whileItem = new QTreeWidgetItem(parent);
    whileItem->setText(0, "WhileStmt（循环语句）");

    // 显示循环条件
    if (whileStmt->condition) {
        QTreeWidgetItem* condItem = new QTreeWidgetItem(whileItem);
        condItem->setText(0, "循环条件：");
        addASTNodeToTree(whileStmt->condition.get(), condItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* noCondItem = new QTreeWidgetItem(whileItem);
        noCondItem->setText(0, "无条件循环");
    }

    // 显示循环体
    if (whileStmt->body) {
        QTreeWidgetItem* bodyItem = new QTreeWidgetItem(whileItem);
        bodyItem->setText(0, "循环体：");
        addASTNodeToTree(whileStmt->body.get(), bodyItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* emptyBodyItem = new QTreeWidgetItem(whileItem);
        emptyBodyItem->setText(0, "循环体为空");
    }
}
void addCompoundStmtNode(CompoundStmt* compound, QTreeWidgetItem* parent) {
    QTreeWidgetItem* compoundItem = new QTreeWidgetItem(parent);
    compoundItem->setText(0, "CompoundStmt（复合语句）");

    // 显示复合语句中的代码块
    if (compound->body) {
        addASTNodeToTree(compound->body.get(), compoundItem);
    } else {
        QTreeWidgetItem* emptyBodyItem = new QTreeWidgetItem(compoundItem);
        emptyBodyItem->setText(0, "复合语句体为空");
    }
}
// 处理 ReturnStmt 节点（return语句，如 return 0;）
void addReturnStmtNode(ReturnStmt* returnStmt, QTreeWidgetItem* parent) {
    QTreeWidgetItem* returnItem = new QTreeWidgetItem(parent);
    returnItem->setText(0, "ReturnStmt（返回语句）");

    // 显示返回值表达式（如果有）
    if (returnStmt->returnValue) {
        QTreeWidgetItem* valueItem = new QTreeWidgetItem(returnItem);
        valueItem->setText(0, "返回值：");
        addASTNodeToTree(returnStmt->returnValue.get(), valueItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* noValueItem = new QTreeWidgetItem(returnItem);
        noValueItem->setText(0, "无返回值（如 return;）");
    }
}


// 处理 ExprStmt 节点（表达式语句，如 printf("hello");）
void addExprStmtNode(ExprStmt* exprStmt, QTreeWidgetItem* parent) {
    QTreeWidgetItem* exprStmtItem = new QTreeWidgetItem(parent);
    exprStmtItem->setText(0, "ExprStmt（表达式语句）");

    // 显示表达式
    if (exprStmt->expr) {
        addASTNodeToTree(exprStmt->expr.get(), exprStmtItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* emptyExprItem = new QTreeWidgetItem(exprStmtItem);
        emptyExprItem->setText(0, "表达式为空（无效）");
    }
}

void addBreakStmtNode(BreakStmt* breakStmt, QTreeWidgetItem* parent) {
    QTreeWidgetItem* breakItem = new QTreeWidgetItem(parent);
    breakItem->setText(0, "BreakStmt（跳转语句）");
}


// 处理Stmt节点（语句：声明、赋值、if、return等）
void addStmtNode(Stmt* stmt, QTreeWidgetItem* parent) {
    if (auto declareStmt = dynamic_cast<DeclareStmt*>(stmt)) {
        addDeclareStmtNode(declareStmt, parent);
    } else if (auto assignStmt = dynamic_cast<AssignStmt*>(stmt)) {
        addAssignStmtNode(assignStmt, parent);
    } else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        addIfStmtNode(ifStmt, parent);
    } else if (auto forStmt = dynamic_cast<ForStmt*>(stmt)) { // 添加ForStmt分支
        addForStmtNode(forStmt, parent);
    }else if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt)) {
        addWhileStmtNode(whileStmt, parent);
    } else if (auto compoundStmt = dynamic_cast<CompoundStmt*>(stmt)) { // 新增复合语句分支
    addCompoundStmtNode(compoundStmt, parent);
    }else if (auto returnStmt = dynamic_cast<ReturnStmt*>(stmt)) {
        addReturnStmtNode(returnStmt, parent);
    } else if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt)) {
        addExprStmtNode(exprStmt, parent);
    } else {
        QTreeWidgetItem* unknownStmtItem = new QTreeWidgetItem(parent);
        unknownStmtItem->setText(0, "未知语句类型");
    }
}
// 处理 BinaryExpr 节点（二元表达式，如 a + b）
void addBinaryExprNode(BinaryExpr* binary, QTreeWidgetItem* parent) {
    QTreeWidgetItem* binaryItem = new QTreeWidgetItem(parent);
    binaryItem->setText(0, QString("BinaryExpr（运算符：%1）").arg(binary->op.c_str()));

    // 显示左操作数
    if (binary->left) {
        QTreeWidgetItem* leftItem = new QTreeWidgetItem(binaryItem);
        leftItem->setText(0, "左操作数：");
        addASTNodeToTree(binary->left.get(), leftItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* invalidLeftItem = new QTreeWidgetItem(binaryItem);
        invalidLeftItem->setText(0, "左操作数为空（无效）");
    }

    // 显示右操作数
    if (binary->right) {
        QTreeWidgetItem* rightItem = new QTreeWidgetItem(binaryItem);
        rightItem->setText(0, "右操作数：");
        addASTNodeToTree(binary->right.get(), rightItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* invalidRightItem = new QTreeWidgetItem(binaryItem);
        invalidRightItem->setText(0, "右操作数为空（无效）");
    }
}
void addUnaryExprNode(UnaryExpr* unary, QTreeWidgetItem* parent) {
    QTreeWidgetItem* unaryItem = new QTreeWidgetItem(parent);
    unaryItem->setText(0, QString("UnaryExpr（运算符：%1%2")
                              .arg(unary->isPostfix ? "" : "")
                              .arg(unary->op.c_str()));

    // 显示操作数
    if (unary->expr) {
        QTreeWidgetItem* exprItem = new QTreeWidgetItem(unaryItem);
        exprItem->setText(0, "操作数：");
        addASTNodeToTree(unary->expr.get(), exprItem);
        QApplication::processEvents();
    } else {
        QTreeWidgetItem* emptyExprItem = new QTreeWidgetItem(unaryItem);
        emptyExprItem->setText(0, "操作数为空（无效）");
    }
}

// 处理 PrimaryExpr 节点（基础表达式：数字、标识符、字符串、括号表达式）
void addPrimaryExprNode(PrimaryExpr* primary, QTreeWidgetItem* parent) {
    QTreeWidgetItem* primaryItem = new QTreeWidgetItem(parent);

    switch (primary->type) {
    case PrimaryExpr::NUMBER:
        primaryItem->setText(0, QString("Number: %1").arg(primary->numberValue.c_str()));
        break;
    case PrimaryExpr::IDENTIFIER:
        primaryItem->setText(0, QString("Identifier: %1").arg(primary->identifier.c_str()));
        break;
    case PrimaryExpr::STRING:
        primaryItem->setText(0, QString("String: \"%1\"").arg(primary->stringValue.c_str()));
        break;
    case PrimaryExpr::CALL_EXPR:
        addCallExprNode(primary->callExpr.get(), parent);
        return; // 注意：直接返回，无需添加primaryItem节点
    case PrimaryExpr::PAREN_EXPR:
        primaryItem->setText(0, "ParenExpr（括号表达式）");
        // 显示括号内的表达式
        if (primary->parenExpr) {
            QTreeWidgetItem* innerExprItem = new QTreeWidgetItem(primaryItem);
            innerExprItem->setText(0, "括号内表达式：");
            addASTNodeToTree(primary->parenExpr.get(), innerExprItem);
            QApplication::processEvents();
        } else {
            QTreeWidgetItem* emptyParenItem = new QTreeWidgetItem(primaryItem);
            emptyParenItem->setText(0, "括号内无表达式（无效）");
        }
        break;
    case PrimaryExpr::UNARY_EXPR:
        addUnaryExprNode(primary->unaryExpr.get(), parent);
        return;
    default:
        primaryItem->setText(0, "未知基础表达式类型");
    }
}

// 处理 Expr 节点（所有表达式的基类，分发到具体表达式类型）
void addExprNode(Expr* expr, QTreeWidgetItem* parent) {
    if (auto binaryExpr = dynamic_cast<BinaryExpr*>(expr)) {
        addBinaryExprNode(binaryExpr, parent);
    } else if (auto unaryExpr = dynamic_cast<UnaryExpr*>(expr)) {
        addUnaryExprNode(unaryExpr, parent);
    } else if (auto primaryExpr = dynamic_cast<PrimaryExpr*>(expr)) {
        addPrimaryExprNode(primaryExpr, parent);
    } else {
        QTreeWidgetItem* unknownExprItem = new QTreeWidgetItem(parent);
        unknownExprItem->setText(0, "未知表达式类型");
    }
}



// 递归构建AST树的入口函数
void addASTNodeToTree(ASTNode* astNode, QTreeWidgetItem* parentItem) {
    if (!astNode || !parentItem) return;  // 空节点直接返回
    qDebug() << "添加节点类型: " << typeid(*astNode).name() << "\n";

    //QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
    //item ->setText(0,astNode->toString())
    // 根据AST节点类型处理
    if (auto program = dynamic_cast<Program*>(astNode)) {
        addProgramNode(program, parentItem);
    }
    else if (auto func = dynamic_cast<FunctionDef*>(astNode)) {
        qDebug() << "处理函数节点: " << func->name << "\n";
        addFunctionDefNode(func, parentItem);
    }
    else if (auto block = dynamic_cast<Block*>(astNode)) {
        qDebug() << "处理Block节点，包含 " << block->statements.size() << " 条语句\n";
        addBlockNode(block, parentItem);
    }
    else if (auto stmt = dynamic_cast<Stmt*>(astNode)) {
         qDebug() << "处理Stmt节点\n";
        addStmtNode(stmt, parentItem);
    }
    else if (auto expr = dynamic_cast<Expr*>(astNode)) {
        //qDebug() << "处理函数节点: " << func->name << "\n";
        addExprNode(expr, parentItem);
    }else{
        QTreeWidgetItem* unknownItem = new QTreeWidgetItem(parentItem);
        unknownItem->setText(0, "未知节点类型");
    }
}

//callespr
void addCallExprNode(CallExpr* callExpr, QTreeWidgetItem* parent) {
    QTreeWidgetItem* callNode = new QTreeWidgetItem(parent);
    callNode->setText(0, "Call Expression");

    // 添加函数名
    QTreeWidgetItem* calleeNode = new QTreeWidgetItem(callNode);
    calleeNode->setText(0, "Callee: " + QString::fromStdString(callExpr->callee));

    // 添加参数列表
    QTreeWidgetItem* argsNode = new QTreeWidgetItem(callNode);
    argsNode->setText(0, "Arguments");
    for (Expr* arg : callExpr->arguments) {
        addExprNode(arg, argsNode);
    }
}
