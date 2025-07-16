#include "CodeHighlighter.h"
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QFont>
// 构造函数：初始化高亮规则
CodeHighlighter::CodeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // 关键字高亮格式
    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);

    // 关键字列表
    QStringList keywordPatterns = {
        "\\bint\\b", "\\bchar\\b", "\\bvoid\\b", "\\breturn\\b",
        "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b", "\\bprintf\\b", "\\bscanf\\b"
    };

    // 添加关键字高亮规则
    for (const QString &pattern : keywordPatterns) {
        highlightingRules.append({
            QRegularExpression(pattern),
            keywordFormat
        });
    }

    // 数字高亮格式
    QTextCharFormat numberFormat;
    numberFormat.setForeground(Qt::darkGreen);
    highlightingRules.append({
        QRegularExpression("\\b\\d+\\b"),
        numberFormat
    });

    // 字符串高亮格式（修复引号匹配问题）
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkMagenta);
    highlightingRules.append({
        QRegularExpression("\\\"[^\\\"]*\\\""),  // 正确匹配双引号字符串
        quotationFormat
    });


    // 单行注释高亮格式
    singleLineCommentFormat.setForeground(Qt::gray);
    highlightingRules.append({
        QRegularExpression("//[^\\n]*"),
        singleLineCommentFormat
    });

    // 函数名高亮格式
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::darkCyan);
    highlightingRules.append({
        QRegularExpression("\\b[A-Za-z0-9_]+(?=\\s*\\()"),
        functionFormat
    });
}

// 重写高亮处理函数
void CodeHighlighter::highlightBlock(const QString &text)
{
    // 应用所有高亮规则
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(
                match.capturedStart(),
                match.capturedLength(),
                rule.format
            );
        }
    }

    // 设置多行注释的高亮（如果需要）
    setCurrentBlockState(0);
}
