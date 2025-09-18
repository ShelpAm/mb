#include <mb/generate-terrain-mesh.h>

#include <mb/perlin.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

std::pair<std::shared_ptr<Mesh>, std::vector<std::vector<float>>>
generate_terrain_mesh(int width, int depth, float scale)
{
    Perlin perlin;
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;

    int rows = depth;
    int cols = width;

    std::vector height(rows + 1, std::vector<float>(cols + 1));

    // 生成顶点 (x, y, z, nx, ny, nz, u, v)
    for (int z = 0; z <= rows; ++z) {
        for (int x = 0; x <= cols; ++x) {
            float xf = static_cast<float>(x);
            float zf = static_cast<float>(z);

            // 使用 Perlin 噪声生成高度
            float y = perlin.noise(xf * scale, zf * scale) * 10.0f * 3.0f;

            // 纹理坐标
            float u = xf / cols; // 归一化到 [0, 1]
            float v = zf / rows;

            vertices.push_back(xf);   // X
            vertices.push_back(y);    // Y (高度)
            vertices.push_back(zf);   // Z
            vertices.push_back(0.0f); // Placeholder: nx
            vertices.push_back(0.0f); // Placeholder: ny
            vertices.push_back(0.0f); // Placeholder: nz
            vertices.push_back(u);    // U
            vertices.push_back(v);    // V

            height[z][x] = y;
        }
    }

    // 计算法向量
    for (int z = 0; z <= rows; ++z) {
        for (int x = 0; x <= cols; ++x) {
            glm::vec3 normal(0.0f);
            float weight = 0.0f;

            if (x < cols && z < rows) { // 右下三角形
                glm::vec3 v0(x, height[z][x], z);
                glm::vec3 v1(x + 1, height[z][x + 1], z);
                glm::vec3 v2(x, height[z + 1][x], z + 1);
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                normal += glm::cross(edge1, edge2);
                weight += 1.0f;
            }
            if (x > 0 && z < rows) { // 左下三角形
                glm::vec3 v0(x, height[z][x], z);
                glm::vec3 v1(x - 1, height[z][x - 1], z);
                glm::vec3 v2(x, height[z + 1][x], z + 1);
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                normal += glm::cross(edge1, edge2);
                weight += 1.0f;
            }
            if (x < cols && z > 0) { // 右上三角形
                glm::vec3 v0(x, height[z][x], z);
                glm::vec3 v1(x + 1, height[z][x + 1], z);
                glm::vec3 v2(x, height[z - 1][x], z - 1);
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                normal += glm::cross(edge1, edge2);
                weight += 1.0f;
            }
            if (x > 0 && z > 0) { // 左上三角形
                glm::vec3 v0(x, height[z][x], z);
                glm::vec3 v1(x - 1, height[z][x - 1], z);
                glm::vec3 v2(x, height[z - 1][x], z - 1);
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                normal += glm::cross(edge1, edge2);
                weight += 1.0f;
            }

            if (weight > 0.0f) {
                normal = glm::normalize(normal / weight);
            }
            else {
                normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            // 更新顶点法向量
            int vertex_idx = (z * (cols + 1) + x) * 8; // 8 floats per vertex
            vertices[vertex_idx + 3] = normal.x;
            vertices[vertex_idx + 4] = normal.y;
            vertices[vertex_idx + 5] = normal.z;
        }
    }

    // 生成索引 (不变)
    for (int z = 0; z < rows; ++z) {
        for (int x = 0; x < cols; ++x) {
            int topLeft = z * (cols + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (cols + 1) + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    return std::make_pair(std::make_shared<Mesh>(vertices, indices), height);
}
