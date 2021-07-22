#ifndef OPTICIO_H
#define OPTICIO_H

#include <QVector>

#define FACENUM 7 // 物面+6个面

enum Color
{
    c, d, f
};

struct Refraction
{
    double c[FACENUM];
    double d[FACENUM];
    double f[FACENUM];
};

struct Face
{
    double radius;		// 半径
    double distance;	// 距离下一个面的距离
    double n;           // 折射率

    double i0;	// 入射角
    double u0;	// 视场角
    double l0;	// 物距

    double i1;	// 入射角
    double u1;	// 视场角
    double l1;	// 像距
};

struct Output
{
    double f=0;                         // 像方焦距
    double l_ideal[3]={};				// 理想像距 | c光 | d光 | f光
    double l_real[3][2]={};             // 实际像位置 | cdf光 | 0.7孔径 1孔径
    double lH=0;						// 像方主面位置
    double lp=0;						// 出瞳距
    double y_ideal[2]={};				// 理想像高 | d光 | 0.7视场 1视场
    double spherical_diff[2]={};		// 球差 0.7孔径 | 1孔径
    double position_diff[3]={};         // 位置色差 | 0孔径 | 0.7孔径 | 1孔径
    double meridian_diff=0;             // 子午场曲 | d光
    double sagittal_diff=0;             // 弧矢场曲 | d光
    double astigmatism=0;				// 像散 | d光
    double y_real[3][2]={};             // 实际像高 | c光 | d光 | f光 | 0.7孔径 | 1孔径
    double distortion_absolute[2]={};	// 绝对畸变 | 0.7视场 | 1视场
    double distortion_relative[2]={};	// 相对畸变 | 0.7视场 | 1视场
    double lateral[2]={};				// 倍率色差 | 0.7视场 | 1视场
    double coma[4]={};					// 子午慧差 | 0.7视场 0.7孔径 | 0.7视场 1孔径 | 1视场 0.7孔径 | 1视场 1孔径
};

struct Input
{
    // 物高 物距 视场角 孔径角 光阑高度
    double H=0;
    double L=0;
    double W=0;
    double U=0;
    double D=0;
    // 折射率
    Refraction ref={};
    // 面
    int faceNum=0;
    Face face[FACENUM]={};
};

#endif // OPTICIO_H
