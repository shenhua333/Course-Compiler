/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *compileButton;
    QPushButton *pushButton;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_2;
    QTextEdit *codeEditor;
    QTreeWidget *astTree;
    QVBoxLayout *verticalLayout;
    QListWidget *tokenList;
    QListWidget *errorList;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(795, 626);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(670, 0, 118, 31));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        compileButton = new QPushButton(layoutWidget);
        compileButton->setObjectName("compileButton");
        compileButton->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(compileButton->sizePolicy().hasHeightForWidth());
        compileButton->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(compileButton);

        pushButton = new QPushButton(layoutWidget);
        pushButton->setObjectName("pushButton");
        sizePolicy1.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());
        pushButton->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(pushButton);

        layoutWidget1 = new QWidget(centralwidget);
        layoutWidget1->setObjectName("layoutWidget1");
        layoutWidget1->setGeometry(QRect(0, 30, 801, 561));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(4);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);
        verticalLayout_2->setContentsMargins(10, 10, 5, 10);
        codeEditor = new QTextEdit(layoutWidget1);
        codeEditor->setObjectName("codeEditor");
        sizePolicy.setHeightForWidth(codeEditor->sizePolicy().hasHeightForWidth());
        codeEditor->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(codeEditor);

        astTree = new QTreeWidget(layoutWidget1);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("AST\346\240\221"));
        astTree->setHeaderItem(__qtreewidgetitem);
        astTree->setObjectName("astTree");
        sizePolicy.setHeightForWidth(astTree->sizePolicy().hasHeightForWidth());
        astTree->setSizePolicy(sizePolicy);

        verticalLayout_2->addWidget(astTree);

        verticalLayout_2->setStretch(0, 6);
        verticalLayout_2->setStretch(1, 4);

        horizontalLayout_2->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(4);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);
        verticalLayout->setContentsMargins(5, 10, 10, 10);
        tokenList = new QListWidget(layoutWidget1);
        tokenList->setObjectName("tokenList");
        sizePolicy.setHeightForWidth(tokenList->sizePolicy().hasHeightForWidth());
        tokenList->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(tokenList);

        errorList = new QListWidget(layoutWidget1);
        errorList->setObjectName("errorList");
        sizePolicy.setHeightForWidth(errorList->sizePolicy().hasHeightForWidth());
        errorList->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(errorList);

        verticalLayout->setStretch(0, 6);
        verticalLayout->setStretch(1, 4);

        horizontalLayout_2->addLayout(verticalLayout);

        horizontalLayout_2->setStretch(0, 7);
        horizontalLayout_2->setStretch(1, 3);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        compileButton->setText(QCoreApplication::translate("MainWindow", "\347\274\226\350\257\221", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "\346\270\205\347\251\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
