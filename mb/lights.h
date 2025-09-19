#pragma once
#include <glm/glm.hpp>

struct Light {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct Directional_light {
    glm::vec3 dir;
};

// Should own Position. But refer to Position to struct Position.
// TODO(shelpam): This must be enforced, but how?
struct Point_light {
    /* glm::vec3 positoin; Refer to this in Position component. */
    float constant;
    float linear;
    float quadratic;
};
