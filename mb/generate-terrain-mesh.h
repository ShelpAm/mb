#pragma once
#include <mb/mesh.h>

#include <memory>

std::pair<std::shared_ptr<Mesh>, std::vector<std::vector<float>>>
generate_terrain_mesh(int width, int depth, float scale);
