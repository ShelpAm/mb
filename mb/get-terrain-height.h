#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

inline float
get_terrain_height(std::vector<std::vector<float>> const &height_map, float x,
                   float z)
{
    // 双线性插值计算高度
    int x0 = static_cast<int>(std::floor(x));
    int z0 = static_cast<int>(std::floor(z));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // 确保索引在mountain_height范围内
    x0 = std::clamp(x0, 0, static_cast<int>(height_map[0].size()) - 1);
    x1 = std::clamp(x1, 0, static_cast<int>(height_map[0].size()) - 1);
    z0 = std::clamp(z0, 0, static_cast<int>(height_map.size()) - 1);
    z1 = std::clamp(z1, 0, static_cast<int>(height_map.size()) - 1);

    // 获取四个邻近格点的高度
    float h00 = height_map[z0][x0];
    float h10 = height_map[z0][x1];
    float h01 = height_map[z1][x0];
    float h11 = height_map[z1][x1];

    // 计算插值权重
    float t = x - x0; // x方向小数部分
    float u = z - z0; // z方向小数部分

    // 双线性插值
    float height = (1.0f - t) * (1.0f - u) * h00 + t * (1.0f - u) * h10 +
                   (1.0f - t) * u * h01 + t * u * h11;
    return height + 2;
}
