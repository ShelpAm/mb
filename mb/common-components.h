#pragma once
#include <mb/mesh.h>
#include <mb/shader-program.h>

#include <glm/glm.hpp>

struct Pathing {
    glm::vec3 destination;
};

struct Position {
    glm::vec3 value;
};

struct Velocity {
    glm::vec3 dir;

    // speed shouldn't be tweaked except that an entity's moving speed has
    // changed. If you want to let some entity stop, you probably want to set
    // dir to {0,0,0}.
    float speed;
};

struct Renderable {
    std::shared_ptr<Mesh> mesh;
    Shader_program const *shader;
    GLuint texture;
};

struct Ai_tag {};

struct Ai_cooldown {
    float timer; // seconds until next decision
    float total;
};

struct Local_player_tag {};
