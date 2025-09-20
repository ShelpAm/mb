#pragma once
#include <mb/mesh.h>

#include <memory>
#include <numeric>

std::pair<std::shared_ptr<Mesh>, std::vector<std::vector<float>>>
generate_terrain_mesh(int width, int depth, float scale);

std::shared_ptr<Mesh> generate_cube_mesh();
