/**
 * uwb_gps_transform.h
 * UWB 局部坐标 → WGS84 经纬度 转换模块
 * 
 * 使用 Helmert 2D 相似变换（旋转+缩放+平移），
 * 标定点 ≥3 时最小二乘拟合，=2 时解析求解，=1 时默认对齐。
 */

#ifndef UWB_GPS_TRANSFORM_H
#define UWB_GPS_TRANSFORM_H

#include <stdint.h>

/* ========== 类型定义 ========== */

/** 标定点：同时具有 UWB 坐标和 GPS 经纬度 */
typedef struct {
    double uwb_x, uwb_y;    /* UWB 局部坐标 (m)，X->南 Y->东 */
    double lat, lon;        /* WGS84 经纬度 (度) */
} uwb_gps_point_t;

/** 变换参数（标定后内部使用） */
typedef struct {
    double a, b;            /* 旋转+缩放参数 */
    double tx, ty;          /* 平移量 (m, ENU) */
    double ref_lat, ref_lon;/* 参考点 GPS（标定点质心） */
    uint8_t calibrated;     /* 是否已完成标定 */
} uwb_gps_transform_t;

extern uwb_gps_transform_t 	xgps;		//转换参数
extern uwb_gps_point_t			xpoint[4];	//定位参数

/* ========== API ========== */

/**
 * 初始化变换句柄
 * @param t  变换句柄指针
 */
void uwb_gps_init(uwb_gps_transform_t *t);

/**
 * 标定：用已知点计算变换参数
 * @param t       变换句柄
 * @param points  标定点数组（至少 1 个）
 * @param n       标定点数量
 * @return 0=成功, -1=点数不足, -2=点共线无法标定
 * 
 * 标定策略：
 *   n=1: 默认对齐模式（UWB X->南, Y->东, 缩放 1:1）
 *   n=2: 解析求解旋转+缩放+平移
 *   n≥3: 最小二乘 Helmert 2D 变换
 */
int uwb_gps_calibrate(uwb_gps_transform_t *t,
                      const uwb_gps_point_t *points, int n);

/**
 * 将 UWB 坐标转换为 GPS 经纬度
 * @param t      已标定的变换句柄
 * @param uwb_x  UWB X 坐标 (m)
 * @param uwb_y  UWB Y 坐标 (m)
 * @param lat    输出：纬度 (度)
 * @param lon    输出：经度 (度)
 */
void uwb_gps_transform(const uwb_gps_transform_t *t,
                       double uwb_x, double uwb_y,
                       double *lat, double *lon);

/**
 * 将 经纬度坐标 转换为 东-北-天坐标 以基站0作为原点
 * @param lat    纬度
 * @param lon    经度
 * @param ret_lat参考纬度
 * @param ret_lon参考经度
 * @param e      输出：东坐标 (米)等效于x轴
 * @param n      输出：北坐标 (米)等效于y轴
 */
void gps_to_enu(double lat, double lon,
                       double ref_lat, double ref_lon,
                       double *e, double *n);
/**
 * 将 东-北-天坐标 转换为 经纬度坐标 以基站0作为原点
 */					   
void enu_to_gps(double e, double n,
                       double ref_lat, double ref_lon,
                       double *lat, double *lon);


#endif /* UWB_GPS_TRANSFORM_H */