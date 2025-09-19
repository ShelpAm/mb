#include <mb/generate-mesh.h>

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

    std::vector<std::vector<float>> height(rows + 1,
                                           std::vector<float>(cols + 1));

    // 生成顶点 (x, y, z, nx, ny, nz, u, v)
    for (int z = 0; z <= rows; ++z) {
        for (int x = 0; x <= cols; ++x) {
            float xf = static_cast<float>(x);
            float zf = static_cast<float>(z);

            // 使用 Perlin 噪声生成高度
            float y = perlin.noise(xf * scale, zf * scale) * 10.0f * 3.0f;

            // 纹理坐标
            float u = xf / cols;
            float v = 1 - zf / rows;

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

    // 计算法向量（基于高度差）
    for (int z = 0; z <= rows; ++z) {
        for (int x = 0; x <= cols; ++x) {
            // 使用高度差计算梯度
            float dx = 0.0f, dz = 0.0f;

            // x方向高度差
            if (x == 0) {
                dx = height[z][x + 1] - height[z][x];
            }
            else if (x == cols) {
                dx = height[z][x] - height[z][x - 1];
            }
            else {
                dx = (height[z][x + 1] - height[z][x - 1]) * 0.5f;
            }

            // z方向高度差
            if (z == 0) {
                dz = height[z + 1][x] - height[z][x];
            }
            else if (z == rows) {
                dz = height[z][x] - height[z - 1][x];
            }
            else {
                dz = (height[z + 1][x] - height[z - 1][x]) * 0.5f;
            }

            // 计算法向量
            glm::vec3 normal(-dx, 1.0f, -dz);
            normal = glm::normalize(normal);

            // 更新顶点法向量
            int vertex_idx = (z * (cols + 1) + x) * 8; // 8 floats per vertex
            vertices[vertex_idx + 3] = normal.x;
            vertices[vertex_idx + 4] = normal.y;
            vertices[vertex_idx + 5] = normal.z;
        }
    }

    // 生成索引
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

std::shared_ptr<Mesh> generate_cube_mesh()
{
    std::vector<float> vertices{
        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // Bottom-right
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // Top-right
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // Top-right
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // Top-left
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // Bottom-left

        // Front face (z = 0.5)
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // Bottom-right
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // Top-right
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // Top-right
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // Top-left
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left

        // Left face (x = -0.5)
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // Top-front
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // Top-back
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom-back
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Bottom-back
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Bottom-front
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // Top-front

        // Right face (x = 0.5)
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // Top-front
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // Top-back
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom-back
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom-back
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Bottom-front
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // Top-front

        // Bottom face (y = -0.5)
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Back-left
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // Back-right
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // Front-right
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // Front-right
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // Front-left
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // Back-left

        // Top face (y = 0.5)
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Back-left
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // Back-right
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // Front-right
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // Front-right
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // Front-left
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // Back-left
    };
    std::vector<std::uint32_t> indices(vertices.size() / 8);
    std::ranges::iota(indices, 0);
    auto cube = std::make_shared<Mesh>(vertices, indices);

    return cube;
}
