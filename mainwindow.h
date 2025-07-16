// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "lexer.h"
#include <QListWidgetItem>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT  // 必须有这个宏！

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //void adjustFontSize(int delta); // 添加字体大小调整函数声明

protected:
    //void wheelEvent(QWheelEvent *event) override; // 添加滚轮事件处理声明

private slots:  // 槽函数部分
    void compileButtonClicked();  // 声明槽函数
    void clearButtonClicked(); //清空函数
    void errorListItemClicked(QListWidgetItem *item);//错误列表
private:
    Ui::MainWindow *ui;
    void displayTokens(const std::vector<Token>& tokens);
    void highlightErrorLine(int line);
    QMap<QListWidgetItem*, int> errorLineMap; // 错误项到行号的映射
    void showError(const QString &errorMsg, int lineNumber,int columnNumber);
    int currentFontSize; // 当前字体大小
};
#endif // MAINWINDOW_H
