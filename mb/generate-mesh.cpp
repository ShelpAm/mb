#include <mb/generate-mesh.h>

#include <mb/model.h>
#include <mb/perlin.h>

#include <glm/glm.hpp>
#include <memory>
#include <vector>

std::pair<std::shared_ptr<Model>, std::vector<std::vector<float>>>
generate_terrain_model(int width, int depth, float scale)
{
    Perlin perlin;
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    Texture diffuse("./resources/wjz.jpg");
    Texture specular("./resources/wjz.jpg");

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

            vertices.push_back(
                {.position = {xf, y, zf}, .normal = {}, .texcoord = {u, v}});

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
            int vertex_idx = (z * (cols + 1)) + x; // 8 floats per vertex
            vertices[vertex_idx].normal = normal;
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

    return std::make_pair(std::make_shared<Model>(vertices, indices,
                                                  std::move(diffuse),
                                                  std::move(specular)),
                          height);
}

std::shared_ptr<Model> generate_cube_model()
{
    std::vector<Vertex> vertices{// Back face (z = -0.5)
                                 {.position = {-0.5f, -0.5f, -0.5f},
                                  .normal = {0.0f, 0.0f, -1.0f},
                                  .texcoord = {0.0f, 0.0f}}, // Bottom-left
                                 {.position = {0.5f, -0.5f, -0.5f},
                                  .normal = {0.0f, 0.0f, -1.0f},
                                  .texcoord = {1.0f, 0.0f}}, // Bottom-right
                                 {.position = {0.5f, 0.5f, -0.5f},
                                  .normal = {0.0f, 0.0f, -1.0f},
                                  .texcoord = {1.0f, 1.0f}}, // Top-right
                                 {.position = {0.5f, 0.5f, -0.5f},
                                  .normal = {0.0f, 0.0f, -1.0f},
                                  .texcoord = {1.0f, 1.0f}}, // Top-right
                                 {.position = {-0.5f, 0.5f, -0.5f},
                                  .normal = {0.0f, 0.0f, -1.0f},
                                  .texcoord = {0.0f, 1.0f}}, // Top-left
                                 {.position = {-0.5f, -0.5f, -0.5f},
                                  .normal = {0.0f, 0.0f, -1.0f},
                                  .texcoord = {0.0f, 0.0f}}, // Bottom-left

                                 // Front face (z = 0.5)
                                 {.position = {-0.5f, -0.5f, 0.5f},
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoord = {0.0f, 0.0f}},
                                 {.position = {0.5f, -0.5f, 0.5f},
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {0.5f, 0.5f, 0.5f},
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoord = {1.0f, 1.0f}},
                                 {.position = {0.5f, 0.5f, 0.5f},
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoord = {1.0f, 1.0f}},
                                 {.position = {-0.5f, 0.5f, 0.5f},
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoord = {0.0f, 1.0f}},
                                 {.position = {-0.5f, -0.5f, 0.5f},
                                  .normal = {0.0f, 0.0f, 1.0f},
                                  .texcoord = {0.0f, 0.0f}},

                                 // Left face (x = -0.5)
                                 {.position = {-0.5f, 0.5f, 0.5f},
                                  .normal = {-1.0f, 0.0f, 0.0f},
                                  .texcoord = {1.0f, 1.0f}},
                                 {.position = {-0.5f, 0.5f, -0.5f},
                                  .normal = {-1.0f, 0.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}},
                                 {.position = {-0.5f, -0.5f, -0.5f},
                                  .normal = {-1.0f, 0.0f, 0.0f},
                                  .texcoord = {0.0f, 0.0f}},
                                 {.position = {-0.5f, -0.5f, -0.5f},
                                  .normal = {-1.0f, 0.0f, 0.0f},
                                  .texcoord = {0.0f, 0.0f}},
                                 {.position = {-0.5f, -0.5f, 0.5f},
                                  .normal = {-1.0f, 0.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {-0.5f, 0.5f, 0.5f},
                                  .normal = {-1.0f, 0.0f, 0.0f},
                                  .texcoord = {1.0f, 1.0f}},

                                 // Right face (x = 0.5)
                                 {.position = {0.5f, 0.5f, 0.5f},
                                  .normal = {1.0f, 0.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}},
                                 {.position = {0.5f, 0.5f, -0.5f},
                                  .normal = {1.0f, 0.0f, 0.0f},
                                  .texcoord = {1.0f, 1.0f}},
                                 {.position = {0.5f, -0.5f, -0.5f},
                                  .normal = {1.0f, 0.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {0.5f, -0.5f, -0.5f},
                                  .normal = {1.0f, 0.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {0.5f, -0.5f, 0.5f},
                                  .normal = {1.0f, 0.0f, 0.0f},
                                  .texcoord = {0.0f, 0.0f}},
                                 {.position = {0.5f, 0.5f, 0.5f},
                                  .normal = {1.0f, 0.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}},

                                 // Bottom face (y = -0.5)
                                 {.position = {-0.5f, -0.5f, -0.5f},
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}},
                                 {.position = {0.5f, -0.5f, -0.5f},
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoord = {1.0f, 1.0f}},
                                 {.position = {0.5f, -0.5f, 0.5f},
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {0.5f, -0.5f, 0.5f},
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {-0.5f, -0.5f, 0.5f},
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoord = {0.0f, 0.0f}},
                                 {.position = {-0.5f, -0.5f, -0.5f},
                                  .normal = {0.0f, -1.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}},

                                 // Top face (y = 0.5)
                                 {.position = {-0.5f, 0.5f, -0.5f},
                                  .normal = {0.0f, 1.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}},
                                 {.position = {0.5f, 0.5f, -0.5f},
                                  .normal = {0.0f, 1.0f, 0.0f},
                                  .texcoord = {1.0f, 1.0f}},
                                 {.position = {0.5f, 0.5f, 0.5f},
                                  .normal = {0.0f, 1.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {0.5f, 0.5f, 0.5f},
                                  .normal = {0.0f, 1.0f, 0.0f},
                                  .texcoord = {1.0f, 0.0f}},
                                 {.position = {-0.5f, 0.5f, 0.5f},
                                  .normal = {0.0f, 1.0f, 0.0f},
                                  .texcoord = {0.0f, 0.0f}},
                                 {.position = {-0.5f, 0.5f, -0.5f},
                                  .normal = {0.0f, 1.0f, 0.0f},
                                  .texcoord = {0.0f, 1.0f}}};
    std::vector<std::uint32_t> indices(vertices.size());
    std::ranges::iota(indices, 0);
    Texture diffuse("./resources/wjz.jpg");
    Texture specular("./resources/wjz.jpg");
    return std::make_shared<Model>(vertices, indices, std::move(diffuse),
                                   std::move(specular));
}
