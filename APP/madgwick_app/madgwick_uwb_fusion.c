/**
 * Madgwick AHRS + UWB天线偏置补偿 实现
 * 适用于 STM32 + LSM6DSL
 */

#include "bsp.h"
#include "madgwick_uwb_fusion.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

// ---- 全局变量 ----
uwb_imu_fusion_t	fusion;
madgwick_t			madgwick;


/* ================================================================
 * 第一部分：Madgwick 姿态解算（核心算法）
 * ================================================================ */

// 快速反平方根（Quake III 经典实现，比 1/sqrtf 快 3x）
static float inv_sqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

void madgwick_update_imu(madgwick_t *m,
                         float gx, float gy, float gz,
                         float ax, float ay, float az)
{
    float q0 = m->q.q0, q1 = m->q.q1, q2 = m->q.q2, q3 = m->q.q3;
    float recip_norm;
    float s0, s1, s2, s3;
    float q_dot0, q_dot1, q_dot2, q_dot3;

    // --- 步骤1：加速度计归一化（得到重力方向单位向量）---
    recip_norm = inv_sqrt(ax * ax + ay * ay + az * az);
    ax *= recip_norm;
    ay *= recip_norm;
    az *= recip_norm;

    // --- 步骤2：梯度下降校正 ---
    // 目标函数：f = [ 2(q1q3 - q0q2) - ax, 2(q0q1 + q2q3) - ay, 2(0.5 - q1² - q2²) - az ]
    // 雅可比矩阵 J 的转置乘 f，得到梯度方向
    // 以下为 Madgwick 论文中公式(21)的展开实现

    // _2q0 = 2*q0, _2q1 = 2*q1, 等等
    float _2q0 = 2.0f * q0;
    float _2q1 = 2.0f * q1;
    float _2q2 = 2.0f * q2;
    float _2q3 = 2.0f * q3;
    float _4q0 = 4.0f * q0;
    float _4q1 = 4.0f * q1;
    float _4q2 = 4.0f * q2;
	float _4q3 = 4.0f * q3;
    float _8q1 = 8.0f * q1;
    float _8q2 = 8.0f * q2;

    // 目标函数值（期望重力方向 vs 实际重力方向）
    float f1 = _2q1 * q3 - _2q0 * q2 - ax;
    float f2 = _2q0 * q1 + _2q2 * q3 - ay;
    float f3 = 1.0f - _2q1 * q1 - _2q2 * q2 - az;

    // J^T * f（梯度），即成本函数对各四元数分量的偏导数
    s0 = -_4q2 * f1 + _4q1 * f2;
    s1 =  _4q3 * f1 + _4q0 * f2 - _8q1 * f3;
    s2 = -_4q0 * f1 + _4q3 * f2 - _8q2 * f3;
    s3 =  _4q1 * f1 + _4q2 * f2;

    // 梯度归一化
    recip_norm = inv_sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    s0 *= recip_norm;
    s1 *= recip_norm;
    s2 *= recip_norm;
    s3 *= recip_norm;

    // --- 步骤3：融合陀螺仪（预测）和加速度计（校正）---
    // q_dot = 0.5 * q ⊗ ω - beta * gradient
    // 其中 ⊗ 是四元数乘法，ω = [0, gx, gy, gz]

    q_dot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - m->beta * s0;
    q_dot1 = 0.5f * ( q0 * gx + q2 * gz - q3 * gy) - m->beta * s1;
    q_dot2 = 0.5f * ( q0 * gy - q1 * gz + q3 * gx) - m->beta * s2;
    q_dot3 = 0.5f * ( q0 * gz + q1 * gy - q2 * gx) - m->beta * s3;

    // --- 步骤4：欧拉积分更新四元数 ---
    float dt = m->inv_sample_freq;
    q0 += q_dot0 * dt;
    q1 += q_dot1 * dt;
    q2 += q_dot2 * dt;
    q3 += q_dot3 * dt;

    // 归一化四元数
    recip_norm = inv_sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    m->q.q0 = q0 * recip_norm;
    m->q.q1 = q1 * recip_norm;
    m->q.q2 = q2 * recip_norm;
    m->q.q3 = q3 * recip_norm;
}

void madgwick_get_euler(const madgwick_t *m, float *roll, float *pitch, float *yaw)
{
    float q0 = m->q.q0, q1 = m->q.q1, q2 = m->q.q2, q3 = m->q.q3;
    
    // roll: 绕X轴
    *roll = atan2f(2.0f * (q0 * q1 + q2 * q3),
                   1.0f - 2.0f * (q1 * q1 + q2 * q2));
    
    // pitch: 绕Y轴
    float sinp = 2.0f * (q0 * q2 - q3 * q1);
    if (sinp >  1.0f) sinp =  1.0f;
    if (sinp < -1.0f) sinp = -1.0f;
    *pitch = asinf(sinp);
    
    // yaw: 绕Z轴
    *yaw = atan2f(2.0f * (q0 * q3 + q1 * q2),
                  1.0f - 2.0f * (q2 * q2 + q3 * q3));
}

float madgwick_get_yaw(const madgwick_t *m) {
    float q0 = m->q.q0, q1 = m->q.q1, q2 = m->q.q2, q3 = m->q.q3;
    // yaw = atan2(2*(q0*q3 + q1*q2), 1 - 2*(q2² + q3²))
    float siny = 2.0f * (q0 * q3 + q1 * q2);
    float cosy = 1.0f - 2.0f * (q2 * q2 + q3 * q3);
    return atan2f(siny, cosy);
}

/* ================================================================
 * 第二部分：天线偏置补偿
 * ================================================================ */

void antenna_comp_init(antenna_compensator_t *comp,
                       const anchor_t *anchors, uint8_t n,
                       const bias_entry_t *table, uint8_t table_sz)
{
    for (uint8_t i = 0; i < n && i < MAX_ANCHORS; i++) {
        comp->anchors[i] = anchors[i];
    }
    comp->n_anchors = n;
    comp->bias_table = table;
    comp->table_size = table_sz;
}

static float lookup_bias(const bias_entry_t *table, uint8_t size, float angle_deg) {
    // 线性插值查表
    if (angle_deg <= table[0].angle_deg)
        return table[0].bias_m;

    for (uint8_t i = 0; i < size - 1; i++) {
        if (angle_deg <= table[i + 1].angle_deg) {
            float t = (angle_deg - table[i].angle_deg) /
                      (table[i + 1].angle_deg - table[i].angle_deg);
            return table[i].bias_m + t * (table[i + 1].bias_m - table[i].bias_m);
        }
    }
    return table[size - 1].bias_m;
}

float antenna_compensate(const antenna_compensator_t *comp,
                         float tag_x, float tag_y, float tag_yaw,
                         uint8_t anchor_id, float raw_dist)
{
    // 1. 找到对应基站
    const anchor_t *anchor = NULL;
    for (uint8_t i = 0; i < comp->n_anchors; i++) {
        if (comp->anchors[i].id == anchor_id) {
            anchor = &comp->anchors[i];
            break;
        }
    }
    if (!anchor) return raw_dist;  // 未知基站，不补偿

    // 2. 计算标签→基站的方位角
    float dx = anchor->x - tag_x;
    float dy = anchor->y - tag_y;
    float anchor_azimuth = atan2f(dy, dx);  // [-π, π]

    // 3. 相对角 = 标签朝向 - 基站方位角
    //    0° 表示标签正面朝向基站（天线增益最大）
    //    180° 表示标签背面朝向基站（天线增益最小）
    float rel_angle = tag_yaw - anchor_azimuth;

    // 归一化到 [0, 2π)
    while (rel_angle < 0) rel_angle += 2.0f * M_PI;
    while (rel_angle >= 2.0f * M_PI) rel_angle -= 2.0f * M_PI;

    // 天线方向图对称：取 [0, π] 范围
    if (rel_angle > M_PI) rel_angle = 2.0f * M_PI - rel_angle;

    float angle_deg = rel_angle * 180.0f / M_PI;

    // 4. 查表补偿
    float bias = lookup_bias(comp->bias_table, comp->table_size, angle_deg);
    return raw_dist - bias;
}

/* ================================================================
 * 第三部分：运动状态检测
 * ================================================================ */

void motion_detect_init(motion_detector_t *md) {
    memset(md->acc_buffer, 0, sizeof(md->acc_buffer));
    md->buf_idx = 0;
    md->buf_count = 0;
    md->var_threshold_static = 0.01f;  // 0.01g²
    md->shock_threshold = 2.0f;        // 2g
}

motion_state_t motion_detect_update(motion_detector_t *md,
                                    float ax, float ay, float az)
{
    // 计算加速度模值
    float acc_mag = sqrtf(ax * ax + ay * ay + az * az);

    // 冲击检测：加速度超过 2g
    if (acc_mag > md->shock_threshold) {
        return MOTION_SHOCK;
    }

    // 更新滑动窗口
    md->acc_buffer[0][md->buf_idx] = ax;
    md->acc_buffer[1][md->buf_idx] = ay;
    md->acc_buffer[2][md->buf_idx] = az;
    md->buf_idx = (md->buf_idx + 1) % 20;
    if (md->buf_count < 20) md->buf_count++;

    // 计算方差
    float mean_x = 0, mean_y = 0, mean_z = 0;
    for (int i = 0; i < md->buf_count; i++) {
        mean_x += md->acc_buffer[0][i];
        mean_y += md->acc_buffer[1][i];
        mean_z += md->acc_buffer[2][i];
    }
    mean_x /= md->buf_count;
    mean_y /= md->buf_count;
    mean_z /= md->buf_count;

    float var = 0;
    for (int i = 0; i < md->buf_count; i++) {
        float dx = md->acc_buffer[0][i] - mean_x;
        float dy = md->acc_buffer[1][i] - mean_y;
        float dz = md->acc_buffer[2][i] - mean_z;
        var += dx * dx + dy * dy + dz * dz;
    }
    var /= md->buf_count;

    // 判定
    if (var < md->var_threshold_static) {
        return MOTION_STATIC;
    } else if (var < 0.2f) {
        return MOTION_SLOW;
    } else {
        return MOTION_FAST;
    }
}

/* ================================================================
 * 第四部分：自适应定位滤波器
 * ================================================================ */

// 简单的最小二乘定位（2D，3基站）
// 返回 0 成功，-1 失败（矩阵奇异）
static int ls_position_2d(const anchor_t *anchors, uint8_t n,
                          const float *compensated_ranges,
                          float *out_x, float *out_y)
{
    if (n < 3) return -1;

    // 以第一个基站为参考
    float x1 = anchors[0].x, y1 = anchors[0].y;
    float r1 = compensated_ranges[0];

    // 实际上 A 的每一行是不同的，修正：
    // 重新计算，每个基站贡献一行
    float A00 = 0, A01 = 0, A10 = 0, A11 = 0;
    float b0 = 0, b1 = 0;

    for (uint8_t i = 1; i < n; i++) {
        float xi = anchors[i].x, yi = anchors[i].y;
        float ri = compensated_ranges[i];
        float d_sq = xi * xi + yi * yi - x1 * x1 - y1 * y1;

        A00 += 4.0f * (xi - x1) * (xi - x1);
        A01 += 4.0f * (xi - x1) * (yi - y1);
        A10 += 4.0f * (xi - x1) * (yi - y1);
        A11 += 4.0f * (yi - y1) * (yi - y1);
        float term = r1 * r1 - ri * ri + d_sq;
        b0 += 2.0f * (xi - x1) * term;
        b1 += 2.0f * (yi - y1) * term;
    }

    float det = A00 * A11 - A01 * A10;
    if (fabsf(det) < 1e-6f) return -1;  // 近奇异，放弃

    *out_x = (A11 * b0 - A01 * b1) / det;
    *out_y = (A00 * b1 - A10 * b0) / det;
    return 0;
}

void uwb_imu_update(uwb_imu_fusion_t *f,
                    const float *ranges, uint8_t n,
                    float gx, float gy, float gz,
                    float ax, float ay, float az,
                    float dt)
{
    // 1. 更新 IMU 姿态
    //    注意：Madgwick 内部用固定采样率，如果 dt 不稳定，需要调 beta
    madgwick_update_imu(&f->madgwick, gx, gy, gz, ax, ay, az);
    f->yaw = madgwick_get_yaw(&f->madgwick);

    // 2. 运动状态检测
    motion_state_t state = motion_detect_update(&f->motion, ax, ay, az);

    // 3. 天线偏置补偿测距值
    float compensated[4];
    for (uint8_t i = 0; i < n && i < MAX_ANCHORS; i++) {
        uint8_t anchor_id = (uint8_t)ranges[i * 2];
        float raw_dist = ranges[i * 2 + 1];
        compensated[i] = antenna_compensate(&f->comp,
                                            f->pos_x, f->pos_y, f->yaw,
                                            anchor_id, raw_dist);
    }

    // 4. 冲击状态：丢弃当前帧
    if (state == MOTION_SHOCK) {
        return;  // 保持上一帧位置
    }

    // 5. 最小二乘定位
    float ls_x, ls_y;
    if (ls_position_2d(f->comp.anchors, n, compensated, &ls_x, &ls_y) < 0) {
        return;  // 矩阵奇异，保持上一帧
    }

    // 6. 自适应滤波（坐标级平滑）
    uint8_t window;
    switch (state) {
        case MOTION_STATIC:  window = 15; break;  // 大窗口强平滑
        case MOTION_SLOW:    window = 5;  break;  // 中等窗口
        case MOTION_FAST:    window = 3;  break;  // 小窗口防滞后
        default:             window = 5;  break;
    }

    // 更新滑动窗口
    f->pos_hist_x[f->hist_idx] = ls_x;
    f->pos_hist_y[f->hist_idx] = ls_y;
    f->hist_idx = (f->hist_idx + 1) % 20;
    if (f->hist_count < 20) f->hist_count++;

    // 取窗口内的均值
    uint8_t actual_window = (window < f->hist_count) ? window : f->hist_count;
    float sum_x = 0, sum_y = 0;
    for (uint8_t i = 0; i < actual_window; i++) {
        uint8_t idx = (f->hist_idx + 20 - 1 - i) % 20;
        sum_x += f->pos_hist_x[idx];
        sum_y += f->pos_hist_y[idx];
    }
    f->pos_x = sum_x / actual_window;
    f->pos_y = sum_y / actual_window;

    // 7. IMU 辅助速度估计（用于丢包时的航位推算）
    //    将加速度从载体坐标系转到世界坐标系，积分得到速度
    //    （省略具体实现，原理：旋转矩阵 × 加速度 → 世界加速度 → 积分）
}