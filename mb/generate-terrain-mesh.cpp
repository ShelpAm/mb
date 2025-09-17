#include <mb/generate-terrain-mesh.h>

#include <mb/perlin.h>

#include <glm/glm.hpp>
#include <vector>

std::shared_ptr<Mesh> generate_terrain_mesh(int width, int depth, float scale)
{
    Perlin perlin;
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;

    int rows = depth;
    int cols = width;

    // 生成顶点 (x, y, z)
    for (int z = 0; z <= rows; ++z) {
        for (int x = 0; x <= cols; ++x) {
            float xf = static_cast<float>(x);
            float zf = static_cast<float>(z);

            // 使用 Perlin noise 生成高度
            float y = perlin.noise(xf * scale, zf * scale) * 10.0f;

            vertices.push_back(xf); // X
            vertices.push_back(y);  // Y (高度)
            vertices.push_back(zf); // Z
        }
    }

    // 生成索引 (两三角形组成一个四边形)
    for (int z = 0; z < rows; ++z) {
        for (int x = 0; x < cols; ++x) {
            int topLeft = z * (cols + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (cols + 1) + x;
            int bottomRight = bottomLeft + 1;

            // 三角形1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // 三角形2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return std::make_shared<Mesh>(vertices, indices);
}
