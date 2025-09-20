#pragma once
#include <algorithm>
#include <cmath>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <numbers>

struct Camera {
    static constexpr glm::vec3 world_up{0, 1, 0};
    float yaw;
    float pitch;
    bool is_active;
    // float roll;

    // Camera(float yaw, float pitch, glm::vec3 pos) : yaw_(yaw), pitch_(pitch)
    // {
    //     assert(std::abs(pitch) < std::numbers::pi / 2);
    // }

    [[nodiscard]] glm::mat4 calc_view_matrix(glm::vec3 position) const
    {
        glm::vec3 dir{std::cos(pitch) * std::cos(yaw), std::sin(pitch),
                      -std::cos(pitch) * std::sin(yaw)};
        auto right = glm::normalize(glm::cross(dir, world_up));
        auto up = glm::normalize(glm::cross(right, dir));
        return glm::lookAt(position, position + dir, up);
    }

    [[nodiscard]] glm::vec3 front() const
    {
        return glm::vec3{std::cos(pitch) * std::cos(yaw), std::sin(pitch),
                         -std::cos(pitch) * std::sin(yaw)};
    }

    [[nodiscard]] glm::vec3 up() const
    {
        glm::vec3 dir{std::cos(pitch) * std::cos(yaw), std::sin(pitch),
                      -std::cos(pitch) * std::sin(yaw)};
        auto right = glm::normalize(glm::cross(dir, world_up));
        return glm::normalize(glm::cross(right, dir));
    }

    [[nodiscard]] glm::vec3 right() const
    {
        glm::vec3 dir{std::cos(pitch) * std::cos(yaw), std::sin(pitch),
                      -std::cos(pitch) * std::sin(yaw)};
        return glm::normalize(glm::cross(dir, world_up));
    }

    void look_at(glm::vec3 position, glm::vec3 target)
    {
        glm::vec3 dir = glm::normalize(target - position);

        // pitch = angle from horizontal plane
        pitch = std::asin(dir.y);

        // yaw = angle on XZ plane (atan2 handles quadrants)
        yaw = std::atan2(-dir.z, dir.x);

        // clamp pitch to avoid gimbal lock
        float max_pitch = (std::numbers::pi / 2) - 0.001F;
        pitch = std::min(pitch, max_pitch);
        pitch = std::max(pitch, -max_pitch);
    }
};
