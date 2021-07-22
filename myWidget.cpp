#include "myWidget.h"

myWidget::myWidget(QWidget *parent)
    : QWidget(parent)
{
    // 初始化
    // 窗口
    this->setWindowTitle("光线追迹");
    this->setWindowIcon(QIcon(":/window/window.png"));
    this->setFixedSize(1200,700);

    this->setAutoFillBackground(true);
    pixmap = new QPixmap(":/optic/optpic/12.png");
    pixmap->scaled(this->size(),Qt::IgnoreAspectRatio);
    palette.setBrush(QPalette::Window, QBrush(*pixmap));
    this->setPalette(palette);

//    opacityEffect=new QGraphicsOpacityEffect;
//    myColor=new QColor(1,1,1);
//    myColor->setAlphaF(0.2);
//    myPalette.setBrush(backgroundRole(),*myColor);

    initInputTable();
    initButton();
    initOutputTable();
    defaultTable();
    showOutputTable();
    initPlot();

    // 信号和槽
    connect(&cal,&QPushButton::clicked,this,&myWidget::calculate);
    connect(&res,&QPushButton::clicked,this,&myWidget::defaultTable);
    connect(&sav,&QPushButton::clicked,this,&myWidget::saveData);
    connect(&ope,&QPushButton::clicked,this,&myWidget::readData);
}

myWidget::~myWidget()
{
}

void myWidget::calculate()
{
    readInput();
    showInputTable();
    // 计算
    outInf=optic.calculateInf(&in);
    outObj=optic.calculateObj(&in);
    calculatedTableInf();
    calculatedTableObj();

    // 像差曲线
    drawSpherical();
    drawDistortion();
}

void myWidget::drawSpherical()
{
    QVector<double> x(51), y(51),yp(51);

    x[0] = 0;
    y[0] = optic.sphercial(&in,0.0001);
    yp[0]=optic.position(&in,0.0001);
    for (int i = 1; i < 51; ++i) {
        x[i] = 1.0/50.0*i; // -1 到 1
        y[i] = optic.sphercial(&in,x[i]);
        yp[i]=optic.position(&in,x[i]);
    }

    sphericalPlot->graph(0)->setData(x, y);
    sphericalPlot->graph(0)->setName("球差");
    sphericalPlot->graph(0)->setPen(QPen(Qt::blue));
    sphericalPlot->graph(1)->setData(x, yp);
    sphericalPlot->graph(1)->setName("位置色差");
    sphericalPlot->graph(1)->setPen(QPen(Qt::green));
    sphericalPlot->legend->setVisible(true);
}

void myWidget::drawDistortion()
{
    QVector<double> x(51), y(51), yp(51),yl(51);
    int e0=1E3;
    int e1=1E3;
    int e2=1E2;
    x[0] = 0;
    y[0] = optic.abDistortion(&in,0.0001)*e0;
    yp[0] = optic.reDistortion(&in,0.0001)*e1;
    yl[0]=optic.lateral(&in,0.0001)*e2;
    for (int i = 1; i < 51; ++i) {
        x[i] = 1.0/50.0*i; // -1 到 1
        y[i] = optic.abDistortion(&in,x[i])*e0;
        yp[i] = optic.reDistortion(&in,x[i])*e1;
        yl[i]=optic.lateral(&in,x[i])*e2;
    }

    distortionPlot->graph(0)->setData(x, y);
    distortionPlot->graph(0)->setName("绝对畸变 10^-3");
    distortionPlot->graph(0)->setPen(QPen(Qt::blue));

    distortionPlot->graph(1)->setData(x, yp);
    distortionPlot->graph(1)->setName("相对畸变 10^-3");
    distortionPlot->graph(1)->setPen(QPen(Qt::green));

    distortionPlot->graph(2)->setData(x, yl);
    distortionPlot->graph(2)->setName("倍率色差 10^-2");
    distortionPlot->graph(2)->setPen(QPen(Qt::red));
    distortionPlot->legend->setVisible(true);
}

void myWidget::initInputTable()
{
    input.setParent(this);
    input.setRowCount(INPUTROW);
    input.setColumnCount(INPUTCOL);
//    input.setPalette(myPalette);
//    input.setAutoFillBackground(true);
//    input.setWindowFlags(Qt::FramelessWindowHint);
//    input.setAttribute(Qt::WA_TranslucentBackground);
//    input.setWindowOpacity(0.5);
//    input.setStyleSheet("background:transparent;");

    // 样式
    input.setStyleSheet("background:rgba(255,255,255,0.5);");
    input.setFrameShape(QFrame::NoFrame);
    input.verticalHeader()->setStyleSheet("background:rgba(255,255,255,0.8);");
    input.horizontalHeader()->setStyleSheet("background:rgba(255,255,255,0.8);");


    for (int i=0; i<INPUTROW; i++)
        for (int j=0; j<INPUTCOL; j++)
        {
            input.setItem(i,j,new QTableWidgetItem);
            input.item(i,j)->setTextAlignment(Qt::AlignRight);
        }

    QStringList header;
    header<<"半径"<<"厚度"<<"c光折射率"<<"d光折射率"<<"f光折射率";
    input.setHorizontalHeaderLabels(header);

    // 设置表格位置和大小
    QPoint tablePos={50,50};
    input.move(tablePos);
    input.horizontalHeader()->setDefaultSectionSize(COLWIDTH);
    input.verticalHeader()->setDefaultSectionSize(ROWHEIGHT);
    double tableWidth=(INPUTCOL+1)*COLWIDTH;
    double tablelHeight=1.7*(INPUTROW+1)*ROWHEIGHT;
    input.setFixedSize(tableWidth,tablelHeight);

    // 物体高度 视场角 孔径角
    double labelHeight=tablelHeight+15+tablePos.y();
    for(int i=0;i<4;i++)
    {
        inputLable[i].setParent(this);
        inputLine[i].setParent(this);
        inputLable[i].move(tablePos.x(),labelHeight+i*30);
        inputLine[i].move(tablePos.x()+70,labelHeight+i*30);
    }
    inputLable[0].setText("物体高度H");
    inputLable[1].setText("光阑直径D");
    inputLable[2].setText("视场角W");
    inputLable[3].setText("孔径角U");

}

void myWidget::initOutputTable()
{
    output.setParent(this);
    output.setRowCount(OUTPUTROW);
    output.setColumnCount(OUTPUTCOL);
    output.setStyleSheet("background:rgba(255,255,255,0.5);");
    for (int i=0; i<OUTPUTROW; i++)
        for (int j=0; j<OUTPUTCOL; j++)
        {
            output.setItem(i,j,new QTableWidgetItem);
            output.item(i,j)->setTextAlignment(Qt::AlignRight);
        }
    output.setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 表格内容
    QStringList header;
    header<<"外部参数"<<"波长"<<"视场角"<<"孔径角"<<"无限远物"<<"有限远物";
    output.setHorizontalHeaderLabels(header);

    // 设置表格位置和大小
    QPoint tablePos={450,50};
    output.move(tablePos);
    output.horizontalHeader()->setDefaultSectionSize(COLWIDTH);
    output.verticalHeader()->setDefaultSectionSize(ROWHEIGHT);
    double tableWidth=(OUTPUTCOL+1)*COLWIDTH;
    double tablelHeight=1.7*(OUTPUTROW-15)*ROWHEIGHT;
    output.setFixedSize(tableWidth,tablelHeight);
    output.setColumnWidth(0,73);
    output.setColumnWidth(1,33);
    output.setColumnWidth(2,43);
    output.setColumnWidth(3,43);
    output.setColumnWidth(4,75);
    output.setColumnWidth(5,75);
}

void myWidget::initButton()
{
    // calculate
    cal.setParent(this);
    cal.setText("calculate");
    cal.move(150,400);

    // default
    res.setParent(this);
    res.setText("default");
    res.move(50,400);

    // save
    sav.setParent(this);
    sav.setText("save");
    sav.move(150,450);

    // open
    ope.setParent(this);
    ope.setText("open");
    ope.move(50,450);
}

void myWidget::initPlot()
{
    sphericalPlot->setStyleSheet("background:rgba(255,255,255,0.5);");
    sphericalPlot->xAxis->setLabel("孔径");
    sphericalPlot->yAxis->setLabel("球差/位置色差");
    sphericalPlot->move(900,50);
    sphericalPlot->setFixedSize(250,250);
    sphericalPlot->xAxis->setRange(0, 1);
    sphericalPlot->yAxis->setRange(-0.5, 0.5);
    sphericalPlot->addGraph();
    sphericalPlot->addGraph();
    sphericalPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);

    distortionPlot->setStyleSheet("background:rgba(255,255,255,0.5);");
    distortionPlot->xAxis->setLabel("视场");
    distortionPlot->yAxis->setLabel("绝对畸变/相对畸变/倍率色差");
    distortionPlot->move(900,350);
    distortionPlot->setFixedSize(250,250);
    distortionPlot->xAxis->setRange(0, 1);
    distortionPlot->yAxis->setRange(-1, 1);
    distortionPlot->addGraph();
    distortionPlot->addGraph();
    distortionPlot->addGraph();
    distortionPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
}

void myWidget::showInputTable()
{
    // 距离 半径 折射率
    for (int i=0; i<in.faceNum; i++)
    {
        input.item(i,0)->setText(QString::number(in.face[i].radius, 'f', PRECISION));
        input.item(i,1)->setText(QString::number(in.face[i].distance, 'f', PRECISION));
        input.item(i,2)->setText(QString::number(in.ref.c[i], 'f', PRECISION));
        input.item(i,3)->setText(QString::number(in.ref.d[i], 'f', PRECISION));
        input.item(i,4)->setText(QString::number(in.ref.f[i], 'f', PRECISION));
    }
    input.item(0,0)->setText("Infinity");

    //物高 光阑直径 视场角 孔径角
    inputLine[0].setText(QString::number(in.H, 'f', PRECISION));
    inputLine[1].setText(QString::number(in.D, 'f', PRECISION));
    inputLine[2].setText(QString::number(in.W, 'f', PRECISION));
    inputLine[3].setText(QString::number(in.U, 'f', PRECISION));
}

void myWidget::showOutputTable()
{
    // 焦距
    output.item(0,0)->setText("焦距f'");
    output.item(0,1)->setText("d");

    // 理想像距
    output.item(1,0)->setText("理想像距l'");
    output.item(1,1)->setText("d");

    output.item(2,1)->setText("C");

    output.item(3,1)->setText("F");

    // 实际像位置
    output.item(4,0)->setText("实际像距l'");
    output.item(4,1)->setText("d");
    output.item(4,2)->setText("0");
    output.item(4,3)->setText("1");

    output.item(5,2)->setText("0");
    output.item(5,3)->setText("0.7");

    output.item(6,1)->setText("C");
    output.item(6,2)->setText("0");
    output.item(6,3)->setText("1");

    output.item(7,2)->setText("0");
    output.item(7,3)->setText("0.7");

    output.item(8,1)->setText("F");
    output.item(8,2)->setText("0");
    output.item(8,3)->setText("1");

    output.item(9,2)->setText("0");
    output.item(9,3)->setText("0.7");

    // 像方主面
    output.item(10,0)->setText("像方主面lH'");
    output.item(10,1)->setText("d");

    // 出瞳距
    output.item(11,0)->setText("出瞳距lp'");
    output.item(11,1)->setText("d");

    // 理想像高
    output.item(12,0)->setText("理想像高y'");
    output.item(12,1)->setText("d");
    output.item(12,2)->setText("1");
    output.item(12,3)->setText("0");

    output.item(13,2)->setText("0.7");
    output.item(13,3)->setText("0");

    // 球差
    output.item(14,0)->setText("球差");
    output.item(14,1)->setText("d");
    output.item(14,2)->setText("0");
    output.item(14,3)->setText("0.7");

    output.item(15,2)->setText("0");
    output.item(15,3)->setText("1");

    // 位置色差
    output.item(16,0)->setText("位置色差");
    output.item(16,1)->setText("F-C");
    output.item(16,2)->setText("0");
    output.item(16,3)->setText("0.7");

    output.item(17,2)->setText("0");
    output.item(17,3)->setText("1");

    output.item(18,2)->setText("0");
    output.item(18,3)->setText("0");

    // 子午场曲
    output.item(19,0)->setText("子午场曲xt'");
    output.item(19,1)->setText("d");
    output.item(19,2)->setText("1");
    output.item(19,3)->setText("0");

    // 弧矢场曲
    output.item(20,0)->setText("弧矢场曲xs'");
    output.item(20,1)->setText("d");
    output.item(20,2)->setText("1");
    output.item(20,3)->setText("0");

    // 弧矢场曲
    output.item(21,0)->setText("像散xts'");
    output.item(21,1)->setText("d");
    output.item(21,2)->setText("1");
    output.item(21,3)->setText("0");

    // 实际像高
    output.item(22,0)->setText("实际像高");
    output.item(22,1)->setText("F");
    output.item(22,2)->setText("0.7");
    output.item(22,3)->setText("0");

    output.item(23,2)->setText("1");
    output.item(23,3)->setText("0");

    output.item(24,1)->setText("d");
    output.item(24,2)->setText("0.7");
    output.item(24,3)->setText("0");

    output.item(25,2)->setText("1");
    output.item(25,3)->setText("0");

    output.item(26,1)->setText("C");
    output.item(26,2)->setText("0.7");
    output.item(26,3)->setText("0");

    output.item(27,2)->setText("1");
    output.item(27,3)->setText("0");

    // 相对畸变
    output.item(28,0)->setText("相对畸变");
    output.item(28,1)->setText("d");
    output.item(28,2)->setText("0.7");

    output.item(29,2)->setText("1");

    // 绝对畸变
    output.item(30,0)->setText("绝对畸变");
    output.item(30,1)->setText("d");
    output.item(30,2)->setText("0.7");

    output.item(31,2)->setText("1");

    // 倍率色差
    output.item(32,0)->setText("倍率色差");
    output.item(32,1)->setText("F-C");
    output.item(32,2)->setText("0.7");
    output.item(32,3)->setText("0");

    output.item(33,2)->setText("1");
    output.item(33,3)->setText("0");

    // 子午慧差
    output.item(34,0)->setText("子午慧差");
    output.item(34,1)->setText("d");
    output.item(34,2)->setText("0.7");
    output.item(34,3)->setText("0.7");

    output.item(35,1)->setText("d");
    output.item(35,2)->setText("0.7");
    output.item(35,3)->setText("1");

    output.item(36,1)->setText("d");
    output.item(36,2)->setText("1");
    output.item(36,3)->setText("0.7");

    output.item(37,1)->setText("d");
    output.item(37,2)->setText("1");
    output.item(37,3)->setText("1");
}

void myWidget::calculatedTableInf()
{
    // 焦距
    output.item(0,4)->setText(QString::number(outInf.f, 'f', PRECISION));

    // 理想像距 d光
    output.item(1,4)->setText(QString::number(outInf.l_ideal[d], 'f', PRECISION));

    // 理想像距 c光
    output.item(2,4)->setText(QString::number(outInf.l_ideal[c], 'f', PRECISION));

    // 理想像距 f光
    output.item(3,4)->setText(QString::number(outInf.l_ideal[f], 'f', PRECISION));

    // 实际像距 d光 1孔径
    output.item(4,4)->setText(QString::number(outInf.l_real[d][1], 'f', PRECISION));

    // 实际像距 d光 0.7孔径
    output.item(5,4)->setText(QString::number(outInf.l_real[d][0], 'f', PRECISION));

    // 实际像距 c光 1孔径
    output.item(6,4)->setText(QString::number(outInf.l_real[c][1], 'f', PRECISION));

    // 实际像距 c光 0.7孔径
    output.item(7,4)->setText(QString::number(outInf.l_real[c][0], 'f', PRECISION));

    // 实际像距 f光 1孔径
    output.item(8,4)->setText(QString::number(outInf.l_real[f][1], 'f', PRECISION));

    // 实际像距 f光 0.7孔径
    output.item(9,4)->setText(QString::number(outInf.l_real[f][0], 'f', PRECISION));

    // 像方主面
    output.item(10,4)->setText(QString::number(outInf.lH, 'f', PRECISION));

    // 出瞳距
    output.item(11,4)->setText(QString::number(outInf.lp, 'f', PRECISION));

    // 理想像高 d光 1视场
    output.item(12,4)->setText(QString::number(outInf.y_ideal[1], 'f', PRECISION));

    // 理想像高 d光 0.7视场
    output.item(13,4)->setText(QString::number(outInf.y_ideal[0], 'f', PRECISION));

    // 球差 d光 0.7孔径
    output.item(14,4)->setText(QString::number(outInf.spherical_diff[0], 'f', PRECISION));

    // 球差 d光 1孔径
    output.item(15,4)->setText(QString::number(outInf.spherical_diff[1], 'f', PRECISION));

    // 位置色差 F-C光 0.7孔径
    output.item(16,4)->setText(QString::number(outInf.position_diff[1], 'f', PRECISION));

    // 位置色差 F-C光 1孔径
    output.item(17,4)->setText(QString::number(outInf.position_diff[2], 'f', PRECISION));

    // 位置色差 F-C光 0孔径
    output.item(18,4)->setText(QString::number(outInf.position_diff[0], 'f', PRECISION));

    // 子午场曲 d光 1视场
    output.item(19,4)->setText(QString::number(outInf.meridian_diff, 'f', PRECISION));

    // 弧矢场曲 d光 1视场
    output.item(20,4)->setText(QString::number(outInf.sagittal_diff, 'f', PRECISION));

    // 像散 d光 1视场
    output.item(21,4)->setText(QString::number(outInf.astigmatism, 'f', PRECISION));

    // 实际像高 F光 0.7视场
    output.item(22,4)->setText(QString::number(outInf.y_real[f][0], 'f', PRECISION));

    // 实际像高 F光 1视场
    output.item(23,4)->setText(QString::number(outInf.y_real[f][1], 'f', PRECISION));

    // 实际像高 d光 0.7视场
    output.item(24,4)->setText(QString::number(outInf.y_real[d][0], 'f', PRECISION));

    // 实际像高 d光 1视场
    output.item(25,4)->setText(QString::number(outInf.y_real[d][1], 'f', PRECISION));

    // 实际像高 C光 0.7视场
    output.item(26,4)->setText(QString::number(outInf.y_real[c][0], 'f', PRECISION));

    // 实际像高 C光 1视场
    output.item(27,4)->setText(QString::number(outInf.y_real[c][1], 'f', PRECISION));

    // 相对畸变 d光 0.7视场
    output.item(28,4)->setText(QString::number(outInf.distortion_relative[0], 'f', PRECISION));

    // 相对畸变 d光 1视场
    output.item(29,4)->setText(QString::number(outInf.distortion_relative[1], 'f', PRECISION));

    // 绝对畸变 d光 0.7视场
    output.item(30,4)->setText(QString::number(outInf.distortion_absolute[0], 'f', PRECISION));

    // 绝对畸变 d光 1视场
    output.item(31,4)->setText(QString::number(outInf.distortion_absolute[1], 'f', PRECISION));

    // 倍率色差 F-C光 0.7视场
    output.item(32,4)->setText(QString::number(outInf.lateral[0], 'f', PRECISION));

    // 倍率色差 F-C光 0.7视场
    output.item(33,4)->setText(QString::number(outInf.lateral[1], 'f', PRECISION));

    // 子午慧差 0.7视场 0.7孔径
    output.item(34,4)->setText(QString::number(outInf.coma[0], 'f', PRECISION));

    // 子午慧差 0.7视场 1.0孔径
    output.item(35,4)->setText(QString::number(outInf.coma[1], 'f', PRECISION));

    // 子午慧差 1.0视场 0.7孔径
    output.item(36,4)->setText(QString::number(outInf.coma[2], 'f', PRECISION));

    // 子午慧差 0.7视场 0.7孔径
    output.item(37,4)->setText(QString::number(outInf.coma[3], 'f', PRECISION));
}

void myWidget::calculatedTableObj()
{
    // 焦距
    output.item(0,5)->setText(QString::number(outObj.f, 'f', PRECISION));

    // 理想像距 d光
    output.item(1,5)->setText(QString::number(outObj.l_ideal[d], 'f', PRECISION));

    // 理想像距 c光
    output.item(2,5)->setText(QString::number(outObj.l_ideal[c], 'f', PRECISION));

    // 理想像距 f光
    output.item(3,5)->setText(QString::number(outObj.l_ideal[f], 'f', PRECISION));

    // 实际像距 d光 1孔径
    output.item(4,5)->setText(QString::number(outObj.l_real[d][1], 'f', PRECISION));

    // 实际像距 d光 0.7孔径
    output.item(5,5)->setText(QString::number(outObj.l_real[d][0], 'f', PRECISION));

    // 实际像距 c光 1孔径
    output.item(6,5)->setText(QString::number(outObj.l_real[c][1], 'f', PRECISION));

    // 实际像距 c光 0.7孔径
    output.item(7,5)->setText(QString::number(outObj.l_real[c][0], 'f', PRECISION));

    // 实际像距 f光 1孔径
    output.item(8,5)->setText(QString::number(outObj.l_real[f][1], 'f', PRECISION));

    // 实际像距 f光 0.7孔径
    output.item(9,5)->setText(QString::number(outObj.l_real[f][0], 'f', PRECISION));

    // 像方主面
    output.item(10,5)->setText(QString::number(outObj.lH, 'f', PRECISION));

    // 出瞳距
    output.item(11,5)->setText(QString::number(outObj.lp, 'f', PRECISION));

    // 理想像高 d光 1视场
    output.item(12,5)->setText(QString::number(outObj.y_ideal[1], 'f', PRECISION));

    // 理想像高 d光 0.7视场
    output.item(13,5)->setText(QString::number(outObj.y_ideal[0], 'f', PRECISION));

    // 球差 d光 0.7孔径
    output.item(14,5)->setText(QString::number(outObj.spherical_diff[0], 'f', PRECISION));

    // 球差 d光 1孔径
    output.item(15,5)->setText(QString::number(outObj.spherical_diff[1], 'f', PRECISION));

    // 位置色差 F-C光 0.7孔径
    output.item(16,5)->setText(QString::number(outObj.position_diff[1], 'f', PRECISION));

    // 位置色差 F-C光 1孔径
    output.item(17,5)->setText(QString::number(outObj.position_diff[2], 'f', PRECISION));

    // 位置色差 F-C光 0孔径
    output.item(18,5)->setText(QString::number(outObj.position_diff[0], 'f', PRECISION));

    // 子午场曲 d光 1视场
    output.item(19,5)->setText(QString::number(outObj.meridian_diff, 'f', PRECISION));

    // 弧矢场曲 d光 1视场
    output.item(20,5)->setText(QString::number(outObj.sagittal_diff, 'f', PRECISION));

    // 像散 d光 1视场
    output.item(21,5)->setText(QString::number(outObj.astigmatism, 'f', PRECISION));

    // 实际像高 F光 0.7视场
    output.item(22,5)->setText(QString::number(outObj.y_real[f][0], 'f', PRECISION));

    // 实际像高 F光 1视场
    output.item(23,5)->setText(QString::number(outObj.y_real[f][1], 'f', PRECISION));

    // 实际像高 d光 0.7视场
    output.item(24,5)->setText(QString::number(outObj.y_real[d][0], 'f', PRECISION));

    // 实际像高 d光 1视场
    output.item(25,5)->setText(QString::number(outObj.y_real[d][1], 'f', PRECISION));

    // 实际像高 C光 0.7视场
    output.item(26,5)->setText(QString::number(outObj.y_real[c][0], 'f', PRECISION));

    // 实际像高 C光 1视场
    output.item(27,5)->setText(QString::number(outObj.y_real[c][1], 'f', PRECISION));

    // 相对畸变 d光 0.7视场
    output.item(28,5)->setText(QString::number(outObj.distortion_relative[0], 'f', PRECISION));

    // 相对畸变 d光 1视场
    output.item(29,5)->setText(QString::number(outObj.distortion_relative[1], 'f', PRECISION));

    // 绝对畸变 d光 0.7视场
    output.item(30,5)->setText(QString::number(outObj.distortion_absolute[0], 'f', PRECISION));

    // 绝对畸变 d光 1视场
    output.item(31,5)->setText(QString::number(outObj.distortion_absolute[1], 'f', PRECISION));

    // 倍率色差 F-C光 0.7视场
    output.item(32,5)->setText(QString::number(outObj.lateral[0], 'f', PRECISION));

    // 倍率色差 F-C光 0.7视场
    output.item(33,5)->setText(QString::number(outObj.lateral[1], 'f', PRECISION));

    // 子午慧差 0.7视场 0.7孔径
    output.item(34,5)->setText(QString::number(outObj.coma[0], 'f', PRECISION));

    // 子午慧差 0.7视场 1.0孔径
    output.item(35,5)->setText(QString::number(outObj.coma[1], 'f', PRECISION));

    // 子午慧差 1.0视场 0.7孔径
    output.item(36,5)->setText(QString::number(outObj.coma[2], 'f', PRECISION));

    // 子午慧差 0.7视场 0.7孔径
    output.item(37,5)->setText(QString::number(outObj.coma[3], 'f', PRECISION));
}

void myWidget::defaultTable()
{
    defaultOptic();
    showInputTable();
}

void myWidget::readInput()
{
    in.faceNum=0;

    // 距离 半径 折射率
    for (int i=0; i<FACENUM; i++)
    {
        double temp=input.item(i,0)->text().toDouble();
        if(temp==0&&i>0)
            break;
        in.face[i].radius=temp;
        in.face[i].distance=input.item(i,1)->text().toDouble();
        in.ref.c[i]=input.item(i,2)->text().toDouble();
        in.ref.d[i]=input.item(i,3)->text().toDouble();
        in.ref.f[i]=input.item(i,4)->text().toDouble();
        in.faceNum++;
    }

    // 物距 物高 光阑直径 视场角 孔径角
    in.L=in.face[0].distance;
    in.H=inputLine[0].text().toDouble();
    in.D=inputLine[1].text().toDouble();
    in.W=inputLine[2].text().toDouble();
    in.U=inputLine[3].text().toDouble();
}

void myWidget::defaultOptic()
{
    in.D=20;
    in.W=3;
    in.H=26;
    in.U=5.0;
    in.L=500;

    in.face[0].distance=500;
    in.face[1].distance=4;
    in.face[2].distance=2.5;

    in.face[0].radius=1.0;
    in.face[1].radius=62.5;
    in.face[2].radius=-43.65;
    in.face[3].radius=-124.35;

    in.ref.c[0]=1;
    in.ref.c[1]=1.5143264578;
    in.ref.c[2]=1.6666173079;
    in.ref.c[3]=1;

    in.ref.d[0]=1;
    in.ref.d[1]=1.5167999978;
    in.ref.d[2]=1.672699817;
    in.ref.d[3]=1;

    in.ref.f[0]=1;
    in.ref.f[1]=1.522376461;
    in.ref.f[2]=1.6875273179;
    in.ref.f[3]=1;

    in.faceNum=4;
}

void myWidget::readData()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("打开文件"),QString("."),tr("Excel Files(*.xlsx)"));    //设置保存的文件名
    QAxObject excel("Excel.Application");//连接Excel控件
    excel.setProperty("Visible", false);// 不显示窗体
    excel.setProperty("DisplayAlerts", false);
    QAxObject* workbooks = excel.querySubObject("WorkBooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", fileName);
    QAxObject* worksheet = workbook->querySubObject("WorkSheets(int)", 1); // 获取工作表集合的工作表1， 即sheet1

    QAxObject* cell;
    QString content;

    // 半径
    for (int i=3; i<=FACENUM+1; i++)
    {
        cell = worksheet->querySubObject("Cells(int, int)", i, 1);  //获单元格值
        content= cell->dynamicCall("text").toString();
        if(!content.isNull())
            in.face[i-2].radius=content.toDouble();
    }
    // 厚度
    for (int i=2; i<=FACENUM+1; i++)
    {
        cell = worksheet->querySubObject("Cells(int, int)", i, 2);  //获单元格值
        content= cell->dynamicCall("text").toString();
        if(!content.isNull())
            in.face[i-2].distance=content.toDouble();
    }
    // c光
    for (int i=2; i<=FACENUM+1; i++)
    {
        cell = worksheet->querySubObject("Cells(int, int)", i, 3);  //获单元格值
        content= cell->dynamicCall("text").toString();
        if(!content.isNull())
            in.ref.c[i-2]=content.toDouble();
    }
    // d光
    for (int i=2; i<=FACENUM+1; i++)
    {
        cell = worksheet->querySubObject("Cells(int, int)", i, 4);  //获单元格值
        content= cell->dynamicCall("text").toString();
        if(!content.isNull())
            in.ref.d[i-2]=content.toDouble();
    }
    // f光
    for (int i=2; i<=FACENUM+1; i++)
    {
        cell = worksheet->querySubObject("Cells(int, int)", i, 5);  //获单元格值
        content= cell->dynamicCall("text").toString();
        if(!content.isNull())
            in.ref.f[i-2]=content.toDouble();
    }

    // 物体高度
    cell = worksheet->querySubObject("Cells(int, int)", 9, 2);  //获单元格值
    content= cell->dynamicCall("text").toString();
    in.H=content.toDouble();

    // 光阑直径
    cell = worksheet->querySubObject("Cells(int, int)", 10, 2);  //获单元格值
    content= cell->dynamicCall("text").toString();
    in.D=content.toDouble();

    // 视场角
    cell = worksheet->querySubObject("Cells(int, int)", 11, 2);  //获单元格值
    content= cell->dynamicCall("text").toString();
    in.W=content.toDouble();

    // 孔径角
    cell = worksheet->querySubObject("Cells(int, int)", 12, 2);  //获单元格值
    content= cell->dynamicCall("text").toString();
    in.U=content.toDouble();

    // 退出
    excel.dynamicCall("Quit()");
    showInputTable();
}

void myWidget::saveData()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("保存文件"),QString("./test.xlsx"),tr("Excel Files(*.xlsx)"));    //设置保存的文件名

    QAxObject *excel = new QAxObject;
    if(excel->setControl("Excel.Application"))
    {
        excel->dynamicCall("SetVisible (bool Visible)",false);
        excel->setProperty("DisplayAlerts",false);
        QAxObject *workbooks = excel->querySubObject("WorkBooks");            //获取工作簿集合
        workbooks->dynamicCall("Add");                                        //新建一个工作簿
        QAxObject *workbook = excel->querySubObject("ActiveWorkBook");        //获取当前工作簿
        QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);
        QAxObject *cell;
        int rowCount = output.rowCount();
        int columnCount = output.columnCount();

        /*添加Excel表头数据*/
        for(int i = 1; i <= columnCount ; i++)
        {
            cell = worksheet->querySubObject("Cells(int,int)", 1, i);
            cell->setProperty("RowHeight", 40);
            cell->setProperty("ColumnWidth", 80);
            cell->dynamicCall("SetValue(const QString&)", output.horizontalHeaderItem(i-1)->data(0).toString());
        }
        /*将列表中的数据保存到Excel文件中*/
        for(int i = 2; i <= rowCount + 1;i++)
        {
            for(int j = 1;j <= 4;j++)
            {
                cell = worksheet->querySubObject("Cells(int,int)", i, j);
                cell->setProperty("NumberFormatLocal","@");
                cell->dynamicCall("SetValue(const QString&)",output.item(i-2,j-1)->text());
            }
        }
        for(int i = 2; i <= rowCount + 1;i++)
        {
            for(int j = 5;j <= columnCount;j++)
            {
                cell = worksheet->querySubObject("Cells(int,int)", i, j);
                cell->setProperty("NumberFormatLocal","0.000000");
                cell->dynamicCall("Value",output.item(i-2,j-1)->text().toDouble());
            }
        }

        /*将生成的Excel文件保存到指定目录下*/
        workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(fileName)); //保存至fileName
        workbook->dynamicCall("Close()");                                                   //关闭工作簿
        excel->dynamicCall("Quit()");                                                       //关闭excel
        delete excel;
        excel = NULL;
    }
}

