/*
 * myMahonyAHRS.cpp
 * 
 * MahonyAHRS algorithm implementation for sensor fusion
 * Based on the algorithm by Sebastian Madgwick
 * 
 * Created: 2025-04-07
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "myMahonyAHRS.h"

namespace myIMU {

// Global variables for MahonyAHRS algorithm
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};  // Quaternion initialized to identity
float myKp = 8.0f;                      // Proportional gain
float myKi = 0.0f;                      // Integral gain
float eInt[3] = {0.0f, 0.0f, 0.0f};     // Integral error

// MahonyAHRS algorithm implementation
void MahonyAHRSupdate(
    float gx, float gy, float gz,     // Gyroscope data in rad/s
    float ax, float ay, float az,     // Accelerometer data (normalized)
    float mx, float my, float mz,     // Magnetometer data (normalized)
    float dt) {                       // Time step in seconds
    
    float norm;
    float hx, hy, hz;
    float bx, bz;
    float vx, vy, vz, wx, wy, wz;
    float ex, ey, ez;
    float qa, qb, qc;

    // 加速度データが有効かチェック
    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        // 加速度ベクトルを正規化
        norm = sqrt(ax * ax + ay * ay + az * az);
        ax /= norm;
        ay /= norm;
        az /= norm;

        // 地磁気データが有効かチェック
        if(!((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))) {
            // 地磁気ベクトルを正規化
            norm = sqrt(mx * mx + my * my + mz * mz);
            mx /= norm;
            my /= norm;
            mz /= norm;

            // 地磁気ベクトルを体軸座標系に変換
            hx = 2.0f * (mx * (0.5f - q[2] * q[2] - q[3] * q[3]) + my * (q[1] * q[2] - q[0] * q[3]) + mz * (q[1] * q[3] + q[0] * q[2]));
            hy = 2.0f * (mx * (q[1] * q[2] + q[0] * q[3]) + my * (0.5f - q[1] * q[1] - q[3] * q[3]) + mz * (q[2] * q[3] - q[0] * q[1]));
            hz = 2.0f * (mx * (q[1] * q[3] - q[0] * q[2]) + my * (q[2] * q[3] + q[0] * q[1]) + mz * (0.5f - q[1] * q[1] - q[2] * q[2]));

            // 地球の地磁気場の基準方向を計算
            bx = sqrt(hx * hx + hy * hy);
            bz = hz;

            // 推定方向の誤差を計算
            wx = 2.0f * bx * (0.5f - q[2] * q[2] - q[3] * q[3]) + 2.0f * bz * (q[1] * q[3] - q[0] * q[2]);
            wy = 2.0f * bx * (q[1] * q[2] - q[0] * q[3]) + 2.0f * bz * (q[0] * q[1] + q[2] * q[3]);
            wz = 2.0f * bx * (q[0] * q[2] + q[1] * q[3]) + 2.0f * bz * (0.5f - q[1] * q[1] - q[2] * q[2]);

            ex = (ay * wz - az * wy);
            ey = (az * wx - ax * wz);
            ez = (ax * wy - ay * wx);
        } else {
            // 地磁気データが無効な場合、加速度のみで誤差を計算
            vx = 2.0f * (q[1] * q[3] - q[0] * q[2]);
            vy = 2.0f * (q[0] * q[1] + q[2] * q[3]);
            vz = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];

            ex = (ay * vz - az * vy);
            ey = (az * vx - ax * vz);
            ez = (ax * vy - ay * vx);
        }

        // 積分誤差を更新
        eInt[0] += ex * myKi * dt;
        eInt[1] += ey * myKi * dt;
        eInt[2] += ez * myKi * dt;

        // 角速度を修正
        gx += myKp * ex + eInt[0];
        gy += myKp * ey + eInt[1];
        gz += myKp * ez + eInt[2];
    }

    // クォータニオンを更新
    qa = q[0];
    qb = q[1];
    qc = q[2];
    q[0] += (-qb * gx - qc * gy - q[3] * gz) * (0.5f * dt);
    q[1] += (qa * gx + qc * gz - q[3] * gy) * (0.5f * dt);
    q[2] += (qa * gy - qb * gz + q[3] * gx) * (0.5f * dt);
    q[3] += (qa * gz + qb * gy - qc * gx) * (0.5f * dt);

    // クォータニオンを正規化
    norm = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    q[0] /= norm;
    q[1] /= norm;
    q[2] /= norm;
    q[3] /= norm;
}

// Initialize the algorithm
void MahonyAHRSinit() {
    // クォータニオンを単位クォータニオンに初期化
    q[0] = 1.0f;
    q[1] = 0.0f;
    q[2] = 0.0f;
    q[3] = 0.0f;
    
    // 積分誤差をゼロに初期化
    eInt[0] = 0.0f;
    eInt[1] = 0.0f;
    eInt[2] = 0.0f;
}

} // namespace myIMU
