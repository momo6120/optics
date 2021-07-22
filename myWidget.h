#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QAxObject>
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>

#include "opticIO.h"
#include "optic.h"
#include "qcustomplot.h"

#define INPUTROW    7
#define INPUTCOL    5
#define OUTPUTROW   38
#define OUTPUTCOL   6
#define ROWHEIGHT   15
#define COLWIDTH    60
#define PRECISION   6 //小数点后有效位数

class myWidget : public QWidget
{
    Q_OBJECT

public:
    myWidget(QWidget *parent = nullptr);
    ~myWidget();

private:
    // 背景
    QPalette palette;
    QPixmap* pixmap;
//    QGraphicsOpacityEffect *opacityEffect;
//    QPalette myPalette;
//    QColor *myColor;

    QTableWidget input;
    QTableWidget output;
    QPushButton cal; // 计算
    QPushButton res; // 默认
    QPushButton sav; // 保存
    QPushButton ope; // 打开
    QLabel inputLable[4];
    QLineEdit inputLine[4];

    Optic optic;

    // 窗口布局
    void initInputTable();
    void initOutputTable();
    void initButton();
    void initPlot();
    void showInputTable();
    void showOutputTable();
    void calculatedTableInf();
    void calculatedTableObj();
    void defaultTable();
    void readInput();

    // 默认初始
    void defaultOptic();

    // 输入输出
    Input in;
    Output outInf;
    Output outObj;

    // 槽函数
    void readData();
    void saveData();
    void calculate();

    // 绘制函数
    QCustomPlot *sphericalPlot = new QCustomPlot(this);
    QCustomPlot *distortionPlot = new QCustomPlot(this);
    void drawSpherical();
    void drawDistortion();
};
#endif // MYWIDGET_H
