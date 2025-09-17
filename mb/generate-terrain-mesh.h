#pragma once
#include <mb/mesh.h>

#include <memory>

std::shared_ptr<Mesh> generate_terrain_mesh(int width, int depth, float scale);
