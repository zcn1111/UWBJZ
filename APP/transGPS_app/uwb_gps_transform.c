/**
 * uwb_gps_transform.c
 * UWB 局部坐标 -> WGS84 经纬度 转换模块（实现）
 *
 * 核心：Helmert 2D 相似变换 + 局部 ENU -> WGS84 互转
 */

#include "uwb_gps_transform.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

uwb_gps_transform_t 	xgps;		//转换参数
uwb_gps_point_t			xpoint[4];	//定位参数

/* ========== 地球常数 ========== */
#define WGS84_A  6378137.0       /* 长半轴 (m) */
#define WGS84_F  0.003352810664  /* 扁率 */
#define WGS84_E2 (WGS84_F * (2.0 - WGS84_F))  /* 第一偏心率平方 */
#define DEG2RAD  (M_PI / 180.0)
#define RAD2DEG  (180.0 / M_PI)

/* ========== 内部函数：GPS <- -> ENU ========== */

/**
 * 计算参考点处的卯酉圈曲率半径和子午圈曲率半径
 */
static void radii(double lat_rad,
                  double *Rn, double *Rm)
{
    double s = sin(lat_rad);
    double w = sqrt(1.0 - WGS84_E2 * s * s);
    *Rn = WGS84_A / w;                     /* 卯酉圈 */
    *Rm = WGS84_A * (1.0 - WGS84_E2) / (w * w * w); /* 子午圈 */
}

/**
 * WGS84 经纬度 -> 局部 ENU（东-北-天）
 * 参考点 ref 为 ENU 原点
 */
void gps_to_enu(double lat, double lon,
                       double ref_lat, double ref_lon,
                       double *e, double *n)
{
    double Rn, Rm;
    radii(ref_lat * DEG2RAD, &Rn, &Rm);

    double dlat = (lat - ref_lat) * DEG2RAD;
    double dlon = (lon - ref_lon) * DEG2RAD;

    *n = dlat * Rm;                        /* 北 (m) */
    *e = dlon * Rn * cos(ref_lat * DEG2RAD); /* 东 (m) */
}

/**
 * 局部 ENU -> WGS84 经纬度
 */
void enu_to_gps(double e, double n,
                       double ref_lat, double ref_lon,
                       double *lat, double *lon)
{
    double Rn, Rm;
    double ref_rad = ref_lat * DEG2RAD;
    radii(ref_rad, &Rn, &Rm);

    *lat = ref_lat + (n / Rm) * RAD2DEG;
    *lon = ref_lon + (e / (Rn * cos(ref_rad))) * RAD2DEG;
}

/* ========== API 实现 ========== */

void uwb_gps_init(uwb_gps_transform_t *t)
{
    t->a = 1.0;
    t->b = 0.0;
    t->tx = 0.0;
    t->ty = 0.0;
    t->ref_lat = 0.0;
    t->ref_lon = 0.0;
    t->calibrated = 0;
}

int uwb_gps_calibrate(uwb_gps_transform_t *t,
                      const uwb_gps_point_t *points, int n)
{
    if (n < 1) return -1;

    /* 参考点：使用所有标定点的均值 */
    double sum_lat = 0.0, sum_lon = 0.0;
    for (int i = 0; i < n; i++) {
        sum_lat += points[i].lat;
        sum_lon += points[i].lon;
    }
    t->ref_lat = sum_lat / n;
    t->ref_lon = sum_lon / n;

    /* ---- n=1: 默认对齐模式 ---- */
    if (n == 1) {
        t->a = 1.0;
        t->b = 0.0;
        t->tx = 0.0;
        t->ty = 0.0;
        t->calibrated = 1;
        return 0;
    }

    /* 将所有点的 GPS 转为 ENU（相对参考点） */
    double ux[16], uy[16], ee[16], nn[16];
    for (int i = 0; i < n; i++) {
        ux[i] = points[i].uwb_x;
        uy[i] = points[i].uwb_y;
        gps_to_enu(points[i].lat, points[i].lon,
                   t->ref_lat, t->ref_lon,
                   &ee[i], &nn[i]);
    }

    /* ---- n=2: 解析解 ---- */
    if (n == 2) {
        double dx = ux[1] - ux[0];
        double dy = uy[1] - uy[0];
        double de = ee[1] - ee[0];
        double dn = nn[1] - nn[0];
        double d2 = dx * dx + dy * dy;

        if (d2 < 1e-12) return -2;  /* 两点重合 */

        t->a = (de * dx + dn * dy) / d2;
        t->b = (dn * dx - de * dy) / d2;
        t->tx = ee[0] - t->a * ux[0] + t->b * uy[0];
        t->ty = nn[0] - t->b * ux[0] - t->a * uy[0];
        t->calibrated = 1;
        return 0;
    }

    /* ---- n≥3: Helmert 2D 最小二乘 ---- */
    double Sx = 0, Sy = 0;
    double Sxx = 0, Syy = 0, Sxy = 0;
    double Se = 0, Sn = 0;
    double Sex = 0, Sey = 0, Snx = 0, Sny = 0;

    for (int i = 0; i < n; i++) {
        Sx  += ux[i];
        Sy  += uy[i];
        Sxx += ux[i] * ux[i];
        Syy += uy[i] * uy[i];
        Sxy += ux[i] * uy[i];
        Se  += ee[i];
        Sn  += nn[i];
        Sex += ee[i] * ux[i];
        Sey += ee[i] * uy[i];
        Snx += nn[i] * ux[i];
        Sny += nn[i] * uy[i];
    }

    double D = n * (Sxx + Syy) - Sx * Sx - Sy * Sy;
    if (fabs(D) < 1e-12) return -2;  /* 标定点共线/重合 */

    t->a  = (n * (Sex + Sny) - Sx * Se - Sy * Sn) / D;
    t->b  = (n * (Snx - Sey) - Sx * Sn + Sy * Se) / D;
    t->tx = (Se - t->a * Sx + t->b * Sy) / n;
    t->ty = (Sn - t->b * Sx - t->a * Sy) / n;

    t->calibrated = 1;
    return 0;
}

void uwb_gps_transform(const uwb_gps_transform_t *t,
                       double uwb_x, double uwb_y,
                       double *lat, double *lon)
{
    /* UWB -> ENU */
    double e = t->a * uwb_x - t->b * uwb_y + t->tx;
    double n = t->b * uwb_x + t->a * uwb_y + t->ty;

    /* ENU -> GPS */
    enu_to_gps(e, n, t->ref_lat, t->ref_lon, lat, lon);
}

/* 经纬度差 -> 局部米坐标 (以基站0为原点) */
void geo_to_local(double lat0, double lon0, 
				   double lat, double lon,
					double *x, double *y)
{
	double c = cos(lat0 * DEG2RAD);
	*y = (lat - lat0) * DEG2RAD * WGS84_A;		//北向
	*x = (lon - lon0) * DEG2RAD * WGS84_A * c;	//东向
}
