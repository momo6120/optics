#include "optic.h"

Optic::Optic()
{

}

Output& Optic::calculateInf(Input* input)
{
    this->input=input;

    cal_inf_ideal_online();
    cal_inf_ideal_offline();
    cal_inf_real_online();
    cal_inf_real_offline();
    cal_inf_field();

    return inf;
}

Output &Optic::calculateObj(Input *input)
{
    this->input=input;

    cal_obj_ideal_online();
    cal_obj_ideal_offline();
    cal_obj_real_online();
    cal_obj_real_offline();
    cal_obj_field();

    return obj;
}

double Optic::sphercial(Input *input, double k_e)
{
    this->input=input;
    return meridion_inf_online(d, k_e)-inf.l_ideal[d];
}

double Optic::position(Input *input, double k_e)
{
    this->input=input;
    return meridion_inf_online(f, k_e)-meridion_inf_online(c, k_e);
}

double Optic::abDistortion(Input *input, double k_w)
{
    this->input=input;
    return meridion_inf_offline(f, 0.0, k_w)-inf.f * tan(k_w * input->W * PI / 180);
}

double Optic::reDistortion(Input *input, double k_w)
{
    this->input=input;
    double temp=inf.f * tan(k_w * input->W * PI / 180);
    return (meridion_inf_offline(f, 0.0, k_w)-temp)/temp;
}

double Optic::lateral(Input *input, double k_w)
{
    this->input=input;
    return meridion_inf_offline(f, 0.0, k_w)-meridion_inf_offline(c, 0.0, k_w);
}

void Optic::calculate_i()
{
    for (int i = 1; i < input->faceNum; i++)
    {
        if (i > 1)
            input->face[i].i0 = (input->face[i].l0 - input->face[i].radius) / input->face[i].radius * input->face[i].u0;

        input->face[i].i1 = input->face[i-1].n / input->face[i].n * input->face[i].i0;
        input->face[i].u1 = input->face[i].i0 + input->face[i].u0 - input->face[i].i1;
        input->face[i].l1 = input->face[i].radius + input->face[i].radius * input->face[i].i1 / input->face[i].u1;

        if (i < input->faceNum - 1)
        {
            input->face[i + 1].l0 = input->face[i].l1 - input->face[i].distance;
            input->face[i + 1].u0 = input->face[i].u1;
        }
    }
}

void Optic::calculate_real_i()
{
    for (int i = 1; i < input->faceNum; i++)
    {
        if (i > 1)
        {
            double sinI0 = (input->face[i].l0 - input->face[i].radius) / input->face[i].radius * sin(input->face[i].u0 * PI / 180);
            input->face[i].i0 = asin(sinI0) * 180 / PI;
        }
        double sinI1 = input->face[i-1].n / input->face[i].n * sin(input->face[i].i0 * PI / 180);
        input->face[i].i1 = asin(sinI1) * 180 / PI;
        input->face[i].u1 = input->face[i].i0 + input->face[i].u0 - input->face[i].i1;
        input->face[i].l1 = input->face[i].radius + input->face[i].radius * sin(input->face[i].i1 * PI / 180) / sin(input->face[i].u1 * PI / 180);

        if (i < input->faceNum - 1)
        {
            input->face[i + 1].l0 = input->face[i].l1 - input->face[i].distance;
            input->face[i + 1].u0 = input->face[i].u1;
        }
    }
}

void Optic::calculate_real()
{
    for (int i = 1; i < input->faceNum; i++)
    {
        double sinI0 = (input->face[i].l0 - input->face[i].radius) / input->face[i].radius * sin(input->face[i].u0 * PI / 180);
        input->face[i].i0 = asin(sinI0) * 180 / PI;
        double sinI1 = input->face[i-1].n / input->face[i].n * sin(input->face[i].i0 * PI / 180);
        input->face[i].i1 = asin(sinI1) * 180 / PI;
        input->face[i].u1 = input->face[i].i0 + input->face[i].u0 - input->face[i].i1;
        input->face[i].l1 = input->face[i].radius + input->face[i].radius * sin(input->face[i].i1 * PI / 180) / sin(input->face[i].u1 * PI / 180);

        if (i < input->faceNum - 1)
        {
            input->face[i + 1].l0 = input->face[i].l1 - input->face[i].distance;
            input->face[i + 1].u0 = input->face[i].u1;
        }
    }
}

void Optic::initLight(Color color)
{
    double* n = nullptr;
    switch (color)
    {
    case c:
        n = input->ref.c;
        break;
    case d:
        n = input->ref.d;
        break;
    case f:
        n = input->ref.f;
        break;
    default:
        return;
        break;
    }
    for (int i = 0; i < input->faceNum; i++)
        input->face[i].n = n[i];
}

void Optic::cal_inf_ideal_online()
{
    input->face[1].u0 = 0;
    input->face[1].l0 = -INFTY;
    input->face[1].i0 = input->D / (2 * input->face[1].radius);

    initLight(d);
    calculate_i();
    // 焦距
    inf.f = input->D / (2 * input->face[input->faceNum - 1].u1);
    // 像方主面
    inf.lH = input->face[input->faceNum - 1].l1 - inf.f;
    // 理想像高 0.7孔径
    inf.y_ideal[0] = inf.f * tan(input->W * 0.7 * PI / 180);
    // 理想像高 1孔径
    inf.y_ideal[1] = inf.f * tan(input->W * PI / 180);
    // d光 理想像距
    inf.l_ideal[d] = input->face[input->faceNum - 1].l1;
    // c光 理想像距
    initLight(c);
    calculate_i();
    inf.l_ideal[c] = input->face[input->faceNum - 1].l1;
    // f光 理想像距
    initLight(f);
    calculate_i();
    inf.l_ideal[f] = input->face[input->faceNum - 1].l1;
}

void Optic::cal_inf_ideal_offline()
{
    input->face[1].u0 = -sin(input->U * PI / 180);
    input->face[1].l0 = 0;

    initLight(d);

    calculate();

    inf.lp = input->face[input->faceNum - 1].l1;
}

void Optic::cal_inf_real_online()
{
    // 实际像距 d光 0.7孔径
    inf.l_real[d][0] = meridion_inf_online(d, 0.7);
    // 实际像距 d光 1孔径
    inf.l_real[d][1] = meridion_inf_online(d, 1.0);
    // 实际像距 c光 0.7孔径
    inf.l_real[c][0] = meridion_inf_online(c, 0.7);
    // 实际像距 c光 1孔径
    inf.l_real[c][1] = meridion_inf_online(c, 1.0);
    // 实际像距 f光 0.7孔径
    inf.l_real[f][0] = meridion_inf_online(f, 0.7);
    // 实际像距 f光 1孔径
    inf.l_real[f][1] = meridion_inf_online(f, 1.0);
    // 球差 d光 0.7孔径
    inf.spherical_diff[0] = inf.l_real[d][0] - inf.l_ideal[d];
    // 球差 d光 1孔径
    inf.spherical_diff[1] = inf.l_real[d][1] - inf.l_ideal[d];
    // 位置色差 0.7孔径
    inf.position_diff[1] = inf.l_real[f][0] - inf.l_real[c][0];
    // 位置色差 1孔径
    inf.position_diff[2] = inf.l_real[f][1] - inf.l_real[c][1];
    // 位置色差 0孔径
    double f0 = meridion_inf_online(f, 0.0001);
    double c0 = meridion_inf_online(c, 0.0001);
    inf.position_diff[0] = f0 - c0;
}

void Optic::cal_inf_real_offline()
{
    // 实际像高------------------------------------------------------------------------------------------------------------
    // 实际像高 d光 0.7视场
    inf.y_real[d][0] = meridion_inf_offline(d, 0.0, 0.7);
    // 实际像高 d光 1视场
    inf.y_real[d][1] = meridion_inf_offline(d, 0.0, 1.0);
    // 实际像高 c光 0.7视场
    inf.y_real[c][0] = meridion_inf_offline(c, 0.0, 0.7);
    // 实际像高 c光 1视场
    inf.y_real[c][1] = meridion_inf_offline(c, 0.0, 1.0);
    // 实际像高 f光 0.7视场
    inf.y_real[f][0] = meridion_inf_offline(f, 0.0, 0.7);
    // 实际像高 f光 1视场
    inf.y_real[f][1] = meridion_inf_offline(f, 0.0, 1.0);

    // 畸变-------------------------------------------------------------------------------
    // 相对畸变 d光 0.7视场
    inf.distortion_relative[0] = (inf.y_real[d][0] - inf.y_ideal[0]) / inf.y_ideal[0];
    // 相对畸变 d光 1视场
    inf.distortion_relative[1] = (inf.y_real[d][1] - inf.y_ideal[1]) / inf.y_ideal[1];
    // 绝对畸变 d光 0.7视场
    inf.distortion_absolute[0] = inf.y_real[d][0] - inf.y_ideal[0];
    // 绝对畸变 d光 1视场
    inf.distortion_absolute[1] = inf.y_real[d][1] - inf.y_ideal[1];

    // 倍率色差-----------------------------------------------------------------------------
    // 倍率色差 f-c光 0.7视场
    inf.lateral[0] = inf.y_real[f][0] - inf.y_real[c][0];
    // 倍率色差 f-c光 1视场
    inf.lateral[1] = inf.y_real[f][1] - inf.y_real[c][1];

    // 子午慧差-------------------------------------------------------------------------------
    inf.coma[0] = coma_inf(0.7, 0.7);
    inf.coma[1] = coma_inf(0.7, 1.0);
    inf.coma[2] = coma_inf(1.0, 0.7);
    inf.coma[3] = coma_inf(1.0, 1.0);
}

void Optic::cal_inf_field()
{
    field_inf();
    inf.meridian_diff = t;
    inf.sagittal_diff = s;
    inf.astigmatism = ast;
}

void Optic::cal_obj_ideal_online()
{
    input->face[1].u0 = -sin(input->U*PI/180);
    input->face[1].l0 = -input->L;

    initLight(d);
    calculate();
    // 焦距
    // result_obj.f = stop.D / (2 * face[faceNum - 1].cal.u1);
    obj.f = inf.f;
    // 像方主面
    // result_obj.lH = face[faceNum - 1].cal.l1 - result_inf.f;
    obj.lH = inf.lH;
    // 理想像高 d光 1孔径
    obj.y_ideal[1] = (-input->face[0].n * input->face[1].u0) / (input->face[input->faceNum - 1].n * input->face[input->faceNum - 1].u1) * input->H;
    // 理想像高 d光 0.7孔径
    obj.y_ideal[0] = obj.y_ideal[1] * 0.7;
    // d光 理想像距
    obj.l_ideal[d] = input->face[input->faceNum - 1].l1;
    // c光 理想像距
    initLight(c);
    calculate();
    obj.l_ideal[c] = input->face[input->faceNum - 1].l1;
    // f光 理想像距
    initLight(f);
    calculate();
    obj.l_ideal[f] = input->face[input->faceNum - 1].l1;
}

void Optic::cal_obj_ideal_offline()
{
    input->face[1].u0 = -sin(input->U*PI/180);
    input->face[1].l0 = 0;

    initLight(d);

    calculate();

    obj.lp = input->face[input->faceNum - 1].l1;
}

void Optic::cal_obj_real_online()
{
    // 实际像距 d光 0.7孔径
    obj.l_real[d][0] = meridion_obj_online(d, 0.7);
    // 实际像距 d光 1孔径
    obj.l_real[d][1] = meridion_obj_online(d, 1.0);
    // 实际像距 c光 0.7孔径
    obj.l_real[c][0] = meridion_obj_online(c, 0.7);
    // 实际像距 c光 1孔径
    obj.l_real[c][1] = meridion_obj_online(c, 1.0);
    // 实际像距 f光 0.7孔径
    obj.l_real[f][0] = meridion_obj_online(f, 0.7);
    // 实际像距 f光 1孔径
    obj.l_real[f][1] = meridion_obj_online(f, 1.0);
    // 球差 d光 0.7孔径
    obj.spherical_diff[0] = obj.l_real[d][0] - obj.l_ideal[d];
    // 球差 d光 1孔径
    obj.spherical_diff[1] = obj.l_real[d][1] - obj.l_ideal[d];
    // 位置色差 0.7孔径
    obj.position_diff[1] = obj.l_real[f][0] - obj.l_real[c][0];
    // 位置色差 1孔径
    obj.position_diff[2] = obj.l_real[f][1] - obj.l_real[c][1];
    // 位置色差 0孔径
    double f0 = meridion_obj_online(f, 0.0001);
    double c0 = meridion_obj_online(c, 0.0001);
    obj.position_diff[0] = f0 - c0;
}

void Optic::cal_obj_real_offline()
{
    // 实际像高------------------------------------------------------------------------------------------------------------
    // 实际像高 d光 0.7视场
    obj.y_real[d][0] = meridion_obj_offline(d, 0.0, 0.7);
    // 实际像高 d光 1视场
    obj.y_real[d][1] = meridion_obj_offline(d, 0.0, 1.0);
    // 实际像高 c光 0.7视场
    obj.y_real[c][0] = meridion_obj_offline(c, 0.0, 0.7);
    // 实际像高 c光 1视场
    obj.y_real[c][1] = meridion_obj_offline(c, 0.0, 1.0);
    // 实际像高 f光 0.7视场
    obj.y_real[f][0] = meridion_obj_offline(f, 0.0, 0.7);
    // 实际像高 f光 1视场
    obj.y_real[f][1] = meridion_obj_offline(f, 0.0, 1.0);

    // 畸变-------------------------------------------------------------------------------
    // 相对畸变 d光 0.7视场
    obj.distortion_relative[0] = (obj.y_real[d][0] - obj.y_ideal[0]) / obj.y_ideal[0];
    // 相对畸变 d光 1视场
    obj.distortion_relative[1] = (obj.y_real[d][1] - obj.y_ideal[1]) / obj.y_ideal[1];
    // 绝对畸变 d光 0.7视场
    obj.distortion_absolute[0] = obj.y_real[d][0] - obj.y_ideal[0];
    // 绝对畸变 d光 1视场
    obj.distortion_absolute[1] = obj.y_real[d][1] - obj.y_ideal[1];

    // 倍率色差-----------------------------------------------------------------------------
    // 倍率色差 f-c光 0.7视场
    obj.lateral[0] = obj.y_real[f][0] - obj.y_real[c][0];
    // 倍率色差 f-c光 1视场
    obj.lateral[1] = obj.y_real[f][1] - obj.y_real[c][1];

    // 子午慧差-------------------------------------------------------------------------------
    obj.coma[0] = coma_obj(0.7, 0.7);
    obj.coma[1] = coma_obj(0.7, 1.0);
    obj.coma[2] = coma_obj(1.0, 0.7);
    obj.coma[3] = coma_obj(1.0, 1.0);
}

void Optic::cal_obj_field()
{
    field_obj();
    obj.meridian_diff = t;
    obj.sagittal_diff = s;
    obj.astigmatism = ast;
}

double Optic::meridion_inf_online(Color color, double k_e)
{
    initLight(color);

    input->face[1].u0 = 0;
    input->face[1].l0 = -INFTY;
    input->face[1].i0 = asin(input->D / (2 * input->face[1].radius) * k_e) * 180 / PI;

    calculate_real_i();

    return input->face[input->faceNum - 1].l1;
}

double Optic::meridion_obj_online(Color color, double k_e)
{
    initLight(color);

    double tanU1 = input->D / 2 / input->L;
    double U1 = atan(tanU1);
    double sinU1 = k_e * sin(U1);

    input->face[1].u0 = asin(sinU1) * 180 / PI;
    input->face[1].l0 = -input->L;

    calculate_real();

    return input->face[input->faceNum - 1].l1;
}

double Optic::meridion_inf_offline(Color color, double k_e, double k_w)
{
    initLight(color);

    // 初始值
    input->face[1].u0 = -input->W * k_w;
    input->face[1].l0 = (input->D / 2 * k_e) / tan(input->W * k_w * PI / 180);

    calculate_real();

    return (input->face[input->faceNum - 1].l1 - inf.l_ideal[d]) * tan(input->face[input->faceNum - 1].u1 / 180 * PI);
}

double Optic::meridion_obj_offline(Color color, double k_e, double k_w)
{
    initLight(color);

    double tanU0 = (k_w * input->H - k_e * input->D / 2) / (input->L);
    input->face[1].u0 = -atan(tanU0) * 180 / PI;
    input->face[1].l0 = k_e * input->D / 2 / tanU0;

    calculate_real();

    return (input->face[input->faceNum - 1].l1 - obj.l_ideal[d]) * tan(input->face[input->faceNum - 1].u1 / 180 * PI);
}

double Optic::coma_inf(double k_e, double k_w)
{
    double ya = meridion_inf_offline(d, k_e, k_w);
    double yb = meridion_inf_offline(d, -k_e, k_w);
    double yp = meridion_inf_offline(d, 0, k_w);
    return ((ya + yb) / 2 - yp);
}

double Optic::coma_obj(double k_e, double k_w)
{
    double ya = meridion_obj_offline(d, k_e, k_w);
    double yb = meridion_obj_offline(d, -k_e, k_w);
    double yp = meridion_obj_offline(d, 0, k_w);
    return ((ya + yb) / 2 - yp);
}

void Optic::field_inf()
{
    double x_o, y_o, x_s;

    double U[FACENUM], U_[FACENUM], I[FACENUM], I_[FACENUM];
    double L[FACENUM] = { 0 }, L_[FACENUM] = { 0 };//distance of object and image,and L[0] is zero
    double rL[FACENUM], rL_[FACENUM];//ideal image and object distance

    double x[FACENUM], h[FACENUM], D[FACENUM];
    double lt_[FACENUM], ls_[FACENUM];
    double xt_[FACENUM], xs_[FACENUM], ast_[FACENUM];//variate related to curvature of field

    /* refractive index of each space(include the object), initial data before each calculation */
    double s[FACENUM], t[FACENUM], s_[FACENUM], t_[FACENUM], ast[FACENUM];//variate related to astigmatism
    double rp[FACENUM] = { 0 }, ndp[FACENUM] = { 0 }, disp[FACENUM] = { 0 };

    x_s = 0;

    U[0] = input->W / 180 * PI;
    x_o = -INFTY;
    y_o = -x_o * tan(U[0]);

    for (int i = 0; i < FACENUM; i++) {
        ndp[i] = 1;
    }

    /* transport data into related arrays */
    initLight(d);

    for (int i = 1; i < input->faceNum; i++) {
        rp[i] = input->face[i].radius;
        ndp[i] = input->face[i].n;
        disp[i] = input->face[i].distance;
    }

    /* calculate initial data */
    t_[0] = s_[0] = 0;
    x[0] = x_o - x_s; //take the distance from x-object to x-stop as x0
    L[1] = -disp[0];
    U[0] = atan(y_o / (-x[0]));
    rL[1] = x[0] - disp[0];

    for (int i = 0; i < input->faceNum; i++) {

        /* calculate x_i+1 */
        I[i] = asin((L[i + 1] - rp[i + 1]) / rp[i + 1] * sin(U[i]));
        I_[i] = asin(ndp[i] / ndp[i + 1] * sin(I[i]));
        U_[i] = U[i] + I[i] - I_[i];
        L_[i + 1] = rp[i + 1] + rp[i + 1] * sin(I_[i]) / sin(U_[i]);

        h[i + 1] = rp[i + 1] * sin(U_[i] + I_[i]);
        x[i + 1] = (L[i] * sin(U[0]) / cos((I[i] - U[0]) / 2)) * (L[i] * sin(U[0]) / cos((I[i] - U[0]) / 2)) / (2 * rp[i + 1]);

        /* calculate s_i+1 & t_i+1 */
        D[i] = (disp[i] - x[i] + x[i + 1]) / cos(U[i]);
        t[i + 1] = t_[i] - D[i];
        s[i + 1] = s_[i] - D[i];

        /* calculate s'_i+1, t'_i+1 and astigmatism */
        t_[i + 1] = ndp[i + 1] * cos(I_[i]) * cos(I_[i]) / ((ndp[i + 1] * cos(I_[i]) - ndp[i] * cos(I[i])) / rp[i + 1] + ndp[i] * cos(I[i]) * cos(I[i]) / t[i + 1]);
        s_[i + 1] = ndp[i + 1] / (ndp[i] / s[i + 1] + (ndp[i + 1] * cos(I_[i]) - ndp[i] * cos(I[i])) / rp[i + 1]);
        ast[i + 1] = (t_[i + 1] - s_[i + 1]) * cos(U_[i]);

        rL_[i + 1] = ndp[i + 1] / ((ndp[i + 1] - ndp[i]) / rp[i + 1] + ndp[i] / rL[i + 1]);
        lt_[i + 1] = t_[i + 1] * cos(U_[i]) + x[i + 1];
        ls_[i + 1] = s_[i + 1] * cos(U_[i]) + x[i + 1];
        xt_[i + 1] = lt_[i + 1] - rL_[i + 1];
        xs_[i + 1] = ls_[i + 1] - rL_[i + 1];
        ast_[i + 1] = xt_[i + 1] - xs_[i + 1];

        /* transmit variable */
        L[i + 2] = L_[i + 1] - disp[i + 1];
        rL[i + 2] = rL_[i + 1] - disp[i + 1];
        U[i + 1] = U_[i];

    }

    this->s = xs_[input->faceNum - 1];
    this->t = xt_[input->faceNum - 1];
    this->ast = ast[input->faceNum - 1];
}

void Optic::field_obj()
{
    double x_o, y_o, x_s;

    double U[FACENUM], U_[FACENUM], I[FACENUM], I_[FACENUM];
    double L[FACENUM] = { 0 }, L_[FACENUM] = { 0 };//distance of object and image,and L[0] is zero
    double rL[FACENUM], rL_[FACENUM];//ideal image and object distance

    double x[FACENUM], h[FACENUM], D[FACENUM];
    double lt_[FACENUM], ls_[FACENUM];
    double xt_[FACENUM], xs_[FACENUM], ast_[FACENUM];//variate related to curvature of field

    /* refractive index of each space(include the object), initial data before each calculation */
    double s[FACENUM], t[FACENUM], s_[FACENUM], t_[FACENUM], ast[FACENUM];//variate related to astigmatism
    double rp[FACENUM] = { 0 }, ndp[FACENUM] = { 0 }, disp[FACENUM] = { 0 };

    x_s = 0;

    U[0] = input->W / 180 * PI;
    x_o = -input->L;
    y_o = -input->H;

    for (int i = 0; i < FACENUM; i++) {
        ndp[i] = 1;
    }

    /* transport data into related arrays */
    initLight(d);

    for (int i = 1; i < input->faceNum; i++) {
        rp[i] = input->face[i].radius;
        ndp[i] = input->face[i].n;
        disp[i] = input->face[i].distance;
    }

    /* calculate initial data */
    t_[0] = s_[0] = 0;
    x[0] = x_o - x_s; //take the distance from x-object to x-stop as x0
    L[1] = -disp[0];
    U[0] = atan(y_o / (-x[0]));
    rL[1] = x[0] - disp[0];

    for (int i = 0; i < input->faceNum; i++) {

        /* calculate x_i+1 */
        I[i] = asin((L[i + 1] - rp[i + 1]) / rp[i + 1] * sin(U[i]));
        I_[i] = asin(ndp[i] / ndp[i + 1] * sin(I[i]));
        U_[i] = U[i] + I[i] - I_[i];
        L_[i + 1] = rp[i + 1] + rp[i + 1] * sin(I_[i]) / sin(U_[i]);

        h[i + 1] = rp[i + 1] * sin(U_[i] + I_[i]);
        x[i + 1] = (L[i] * sin(U[0]) / cos((I[i] - U[0]) / 2)) * (L[i] * sin(U[0]) / cos((I[i] - U[0]) / 2)) / (2 * rp[i + 1]);

        /* calculate s_i+1 & t_i+1 */
        D[i] = (disp[i] - x[i] + x[i + 1]) / cos(U[i]);
        t[i + 1] = t_[i] - D[i];
        s[i + 1] = s_[i] - D[i];

        /* calculate s'_i+1, t'_i+1 and astigmatism */
        t_[i + 1] = ndp[i + 1] * cos(I_[i]) * cos(I_[i]) / ((ndp[i + 1] * cos(I_[i]) - ndp[i] * cos(I[i])) / rp[i + 1] + ndp[i] * cos(I[i]) * cos(I[i]) / t[i + 1]);
        s_[i + 1] = ndp[i + 1] / (ndp[i] / s[i + 1] + (ndp[i + 1] * cos(I_[i]) - ndp[i] * cos(I[i])) / rp[i + 1]);
        ast[i + 1] = (t_[i + 1] - s_[i + 1]) * cos(U_[i]);

        rL_[i + 1] = ndp[i + 1] / ((ndp[i + 1] - ndp[i]) / rp[i + 1] + ndp[i] / rL[i + 1]);
        lt_[i + 1] = t_[i + 1] * cos(U_[i]) + x[i + 1];
        ls_[i + 1] = s_[i + 1] * cos(U_[i]) + x[i + 1];
        xt_[i + 1] = lt_[i + 1] - rL_[i + 1];
        xs_[i + 1] = ls_[i + 1] - rL_[i + 1];
        ast_[i + 1] = xt_[i + 1] - xs_[i + 1];

        /* transmit variable */
        L[i + 2] = L_[i + 1] - disp[i + 1];
        rL[i + 2] = rL_[i + 1] - disp[i + 1];
        U[i + 1] = U_[i];

    }

    this->s = xs_[input->faceNum - 1];
    this->t = xt_[input->faceNum - 1];
    this->ast = ast[input->faceNum - 1];
}

void Optic::calculate()
{
    for (int i = 1; i < input->faceNum; i++)
    {
        input->face[i].i0 = (input->face[i].l0 - input->face[i].radius) / input->face[i].radius * input->face[i].u0;
        input->face[i].i1 = input->face[i-1].n / input->face[i].n * input->face[i].i0;
        input->face[i].u1 = input->face[i].i0 + input->face[i].u0 - input->face[i].i1;
        input->face[i].l1 = input->face[i].radius + input->face[i].radius * input->face[i].i1 / input->face[i].u1;
        if (i < input->faceNum - 1) {
            input->face[i + 1].l0 = input->face[i].l1 - input->face[i].distance;
            input->face[i + 1].u0 = input->face[i].u1;
        }
    }
}
