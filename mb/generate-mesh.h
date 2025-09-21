#pragma once
#include <mb/mesh.h>

#include <memory>
#include <numeric>

class Model;

std::pair<std::shared_ptr<Model>, std::vector<std::vector<float>>>
generate_terrain_model(int width, int depth, float scale);

std::shared_ptr<Model> generate_cube_model();
