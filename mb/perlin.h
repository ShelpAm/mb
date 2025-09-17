// AI Generated
// 简单 Perlin 噪声实现
// 参考 Ken Perlin 原始算法

#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <glm/glm.hpp>
#include <numeric>
#include <random>
#include <vector>

class Perlin {
  public:
    Perlin()
    {
        // 初始化随机置换表
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::shuffle(p.begin(), p.end(), std::mt19937{std::random_device{}()});
        p.insert(p.end(), p.begin(), p.end()); // 512
    }

    float noise(float x, float y, float z = 0.0f) const
    {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        return lerp(
            w,
            lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                 lerp(u, grad(p[AB], x, y - 1, z),
                      grad(p[BB], x - 1, y - 1, z))),
            lerp(v,
                 lerp(u, grad(p[AA + 1], x, y, z - 1),
                      grad(p[BA + 1], x - 1, y, z - 1)),
                 lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                      grad(p[BB + 1], x - 1, y - 1, z - 1))));
    }

  private:
    std::vector<int> p;

    static float fade(float t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    static float lerp(float t, float a, float b)
    {
        return a + t * (b - a);
    }
    static float grad(int hash, float x, float y, float z)
    {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
};
