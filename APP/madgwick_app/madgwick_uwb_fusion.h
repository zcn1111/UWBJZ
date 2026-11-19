/**
 * Madgwick AHRS + UWB天线偏置补偿 框架
 * 适用于 STM32 + LSM6DSL
 * 作者: Claw
 * 日期: 2026-08-07
 */

#ifndef MADGWICK_UWB_FUSION_H
#define MADGWICK_UWB_FUSION_H

#include <math.h>
#include <stdint.h>

/* ================================================================
 * 第一部分：Madgwick 姿态解算
 * ================================================================ */

// 四元数结构体
typedef struct {
    float q0, q1, q2, q3;  // w, x, y, z
} quaternion_t;

// 欧拉角（弧度）
typedef struct {
    float roll;   // 绕X轴
    float pitch;  // 绕Y轴
    float yaw;    // 绕Z轴（偏航角，即标签朝向）
} euler_t;

// Madgwick 滤波器状态
typedef struct {
    quaternion_t q;          // 当前姿态四元数
    float beta;              // 梯度下降步长（算法增益）
    float sample_freq;       // 采样频率 Hz
    float inv_sample_freq;   // 1/采样频率
} madgwick_t;

/**
 * 初始化 Madgwick 滤波器
 * @param m        滤波器实例
 * @param beta     梯度下降增益，推荐 0.03~0.1。beta越大收敛越快但对振动敏感
 * @param freq_hz  IMU 采样频率，如 100.0f
 */
static inline void madgwick_init(madgwick_t *m, float beta, float freq_hz) {
    m->q.q0 = 1.0f; // 初始姿态：无旋转
    m->q.q1 = 0.0f;
    m->q.q2 = 0.0f;
    m->q.q3 = 0.0f;
    m->beta = beta;
    m->sample_freq = freq_hz;
    m->inv_sample_freq = 1.0f / freq_hz;
}

/**
 * Madgwick 姿态更新（仅加速度计 + 陀螺仪，无磁力计）
 * 
 * 算法原理：
 * 1. 陀螺仪积分 → 预测四元数
 * 2. 加速度计测量重力方向 → 梯度下降求最优四元数
 * 3. 两者融合（互补滤波思想）
 * 
 * @param m       滤波器实例
 * @param gx,gy,gz  陀螺仪角速度 (rad/s)
 * @param ax,ay,az  加速度计测量值 (任意单位，函数内部归一化)
 */
void madgwick_update_imu(madgwick_t *m, 
                         float gx, float gy, float gz,
                         float ax, float ay, float az);

/**
 * 从四元数提取偏航角（标签朝向）
 * @return 偏航角，弧度，范围 [-π, π]
 */
float madgwick_get_yaw(const madgwick_t *m);
void madgwick_get_euler(const madgwick_t *m, float *roll, float *pitch, float *yaw);

/* ================================================================
 * 第二部分：天线偏置补偿
 * ================================================================ */

// 基站信息
#define MAX_ANCHORS 4

typedef struct {
    float x, y;       // 基站坐标 (m)
    float id;         // 基站ID
} anchor_t;

// 角度-偏置标定表（离线标定获得）
// 格式：{角度范围上限(度), 偏置补偿量(m)}
// 偏置补偿量 = 测距值 - 真实距离（正值表示测距偏大，需减去）
typedef struct {
    float angle_deg;   // 角度上限
    float bias_m;       // 对应偏置补偿量
} bias_entry_t;

// 天线补偿器
typedef struct {
    anchor_t anchors[MAX_ANCHORS];
    uint8_t n_anchors;
    const bias_entry_t *bias_table;  // 指向标定表
    uint8_t table_size;
} antenna_compensator_t;

/**
 * 初始化天线补偿器
 * @param comp      补偿器实例
 * @param anchors   基站坐标数组
 * @param n         基站数量
 * @param table     角度-偏置标定表
 * @param table_sz  标定表条目数
 */
void antenna_comp_init(antenna_compensator_t *comp,
                       const anchor_t *anchors, uint8_t n,
                       const bias_entry_t *table, uint8_t table_sz);

/**
 * 补偿单个基站的测距值
 * 
 * 步骤：
 * 1. 计算标签→基站的方位角
 * 2. 结合标签朝向（yaw）得到相对角
 * 3. 查表获取偏置补偿量
 * 4. 返回补偿后的测距值
 * 
 * @param comp      补偿器
 * @param tag_x,y   标签当前估计位置
 * @param tag_yaw   标签当前朝向（rad，来自 Madgwick）
 * @param anchor_id 基站ID
 * @param raw_dist  原始测距值 (m)
 * @return          补偿后的测距值 (m)
 */
float antenna_compensate(const antenna_compensator_t *comp,
                         float tag_x, float tag_y, float tag_yaw,
                         uint8_t anchor_id, float raw_dist);

/* ================================================================
 * 第三部分：运动状态检测
 * ================================================================ */

typedef enum {
    MOTION_STATIC = 0,   // 静止
    MOTION_SLOW,         // 慢速移动
    MOTION_FAST,         // 快速移动
    MOTION_SHOCK         // 冲击/碰撞
} motion_state_t;

typedef struct {
    float acc_buffer[3][20];  // 加速度滑动窗口 [轴][历史]
    uint8_t buf_idx;
    uint8_t buf_count;
    float var_threshold_static;  // 静态判定方差阈值，默认 0.01g²
    float shock_threshold;       // 冲击判定阈值，默认 2.0g
} motion_detector_t;

void motion_detect_init(motion_detector_t *md);
motion_state_t motion_detect_update(motion_detector_t *md,
                                    float ax, float ay, float az);

/* ================================================================
 * 第四部分：自适应定位滤波器
 * ================================================================ */

typedef struct {
    float pos_x, pos_y;         // 位置
    float vel_x, vel_y;         // 速度（来自IMU积分）
    float yaw;                  // 朝向
    madgwick_t madgwick;
    antenna_compensator_t comp;
    motion_detector_t motion;
    
    // 坐标级滑动平均缓冲
    float pos_hist_x[20];
    float pos_hist_y[20];
    uint8_t hist_idx;
    uint8_t hist_count;
} uwb_imu_fusion_t;

/**
 * 主循环：每收到一组 UWB 测距数据时调用
 * 
 * @param f      融合器
 * @param ranges 测距数组 [基站ID, 测距值, ...]，长度 n_anchors*2
 * @param n      测距组数
 * @param gx,gy,gz 陀螺仪 (rad/s)
 * @param ax,ay,az 加速度计 (m/s²)
 * @param dt     距上次调用时间 (s)
 */
void uwb_imu_update(uwb_imu_fusion_t *f,
                    const float *ranges, uint8_t n,
                    float gx, float gy, float gz,
                    float ax, float ay, float az,
                    float dt);

// ---- 全局变量 ----					
extern uwb_imu_fusion_t	fusion;
extern madgwick_t			madgwick;


#endif /* MADGWICK_UWB_FUSION_H */