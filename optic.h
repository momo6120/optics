#ifndef OPTIC_H
#define OPTIC_H

#include "opticIO.h"
#include <cmath>
#define INFTY 1E15
#define PI 3.141592

class Optic
{
public:
    Optic();
    Output& calculateInf(Input* input);
    Output& calculateObj(Input* input);

    // 球差计算
    double sphercial(Input *input,double k_e);
    double position(Input *input,double k_e);
    double abDistortion(Input *input,double k_w);
    double reDistortion(Input *input,double k_w);
    double lateral(Input *input,double k_w);
private:
    Input* input;
    Output inf={};
    Output obj={};

    // 场曲
    double s = 0;
    double t = 0;
    double ast = 0;

    // 无限远
    void cal_inf_ideal_online();
    void cal_inf_ideal_offline();
    void cal_inf_real_online();
    void cal_inf_real_offline();
    void cal_inf_field();

    // 有限远
    void cal_obj_ideal_online();
    void cal_obj_ideal_offline();
    void cal_obj_real_online();
    void cal_obj_real_offline();
    void cal_obj_field();

    double meridion_inf_online(Color color, double k_e);
    double meridion_obj_online(Color color, double k_e);
    double meridion_inf_offline(Color color, double k_e, double k_w);
    double meridion_obj_offline(Color color, double k_e, double k_w);
    double coma_inf(double k_e, double k_w);
    double coma_obj(double k_e, double k_w);

    void field_inf();
    void field_obj();

    void calculate();
    void calculate_i();
    void calculate_real_i();
    void calculate_real();

    void initLight(Color color);
};

#endif // OPTIC_H
