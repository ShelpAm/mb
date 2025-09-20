#include <mb/systems.h>

#include <mb/components.h>
#include <mb/game.h>
#include <mb/lights.h>
#include <mb/mesh.h>
#include <mb/shader-program.h>

#include <random>
// FIXME: Should be removed: ECS shouldn't depend on particular glfwTime.
#include <GLFW/glfw3.h>

void movement_system(entt::registry &reg, float dt,
                     std::vector<std::vector<float>> const &mountain_height)
{
    // Grants velocity to those who have will to pathing to somewhere.
    constexpr double eps{0.5};
    auto pathings = reg.view<Pathing, Position, Velocity>();
    for (auto [e, pathing, pos, vel] : pathings.each()) {
        // Pathing to x,z
        if (glm::distance(
                glm::vec2{pathing.destination.x, pathing.destination.z},
                glm::vec2{pos.value.x, pos.value.z}) > eps) {
            spdlog::debug("pathing: {} -> ({}, {}, {})", static_cast<int>(e),
                          pos.value.x, pos.value.y, pos.value.z);
            vel.dir = glm::normalize(pathing.destination - pos.value);
        }
        else {
            spdlog::debug("pathing: {} arrived ({}, {}, {})",
                          static_cast<int>(e), pos.value.x, pos.value.y,
                          pos.value.z);
            vel.dir = {};
            reg.remove<Pathing>(e);
        }
    }

    // Simulates movement of sun
    auto dlights = reg.view<Directional_light>();
    for (auto [entity, dlight] : dlights.each()) {
        // Simulate sun movement (circular arc in x-y plane)
        float angle = glfwGetTime() * 0.5f; // Adjust speed (0.5 radians/sec)
        dlight.dir = glm::normalize(
            glm::vec3(cos(angle), -sin(angle), sin(angle) * 0.5f));
    }

    // Moves those have velocity to their direction.
    auto moveables = reg.view<Position, Velocity>().each();
    for (auto [entity, pos, vel] : moveables) {
        if (glm::length(vel.dir) < 1e-5) { // Regarded as still
            continue;
        }
        pos.value += glm::normalize(vel.dir) * vel.speed * dt;

        if (reg.all_of<Army>(entity) || reg.all_of<Fps_camemra_tag>(entity)) {
            // 双线性插值计算高度
            float x = pos.value.x;
            float z = pos.value.z;
            int x0 = static_cast<int>(std::floor(x));
            int z0 = static_cast<int>(std::floor(z));
            int x1 = x0 + 1;
            int z1 = z0 + 1;

            // 确保索引在mountain_height范围内
            x0 = std::clamp(x0, 0,
                            static_cast<int>(mountain_height[0].size()) - 1);
            x1 = std::clamp(x1, 0,
                            static_cast<int>(mountain_height[0].size()) - 1);
            z0 =
                std::clamp(z0, 0, static_cast<int>(mountain_height.size()) - 1);
            z1 =
                std::clamp(z1, 0, static_cast<int>(mountain_height.size()) - 1);

            // 获取四个邻近格点的高度
            float h00 = mountain_height[z0][x0];
            float h10 = mountain_height[z0][x1];
            float h01 = mountain_height[z1][x0];
            float h11 = mountain_height[z1][x1];

            // 计算插值权重
            float t = x - x0; // x方向小数部分
            float u = z - z0; // z方向小数部分

            // 双线性插值
            float height = (1.0f - t) * (1.0f - u) * h00 +
                           t * (1.0f - u) * h10 + (1.0f - t) * u * h01 +
                           t * u * h11;

            pos.value.y = height + 2.0f; // 保持你的+2偏移
        }

        auto p = pos.value;
        spdlog::debug("entity {} pos={},{},{}",
                      static_cast<std::size_t>(entity), p.x, p.y, p.z);
    }

    // Simulates 手电筒灯光 (跟随摄像机)
    auto slights = reg.view<Light, Spot_light, Position>();
    auto cameras = reg.view<Camera, View_mode>();
    for (auto [entity, light, slight, pos] : slights.each()) {
        for (auto [cam_entity, cam, view_mode] : cameras.each()) {
            if (view_mode == View_mode::First_player) {
                pos.value = reg.get<Position>(cam_entity).value;
                slight.dir = reg.get<Camera>(cam_entity).front();
            }
        }
    }
}

void collision_system(entt::registry &registry, float now)
{
    // Pairs in this map won't collide until `value`.
    // first entity should be less than second entity.
    static std::map<std::pair<entt::entity, entt::entity>, double>
        collision_free;

    // Removes pairs that can perform collision between them.
    for (auto it = collision_free.begin(); it != collision_free.end();) {
        if (now >= it->second) {
            spdlog::debug("Removing collision_free pair <{} {}>",
                          static_cast<int>(it->first.first),
                          static_cast<int>(it->first.second));
            it = collision_free.erase(it);
        }
        else {
            ++it;
        }
    }

    // TODO FIXME
    auto armies = registry.view<Army, Position>();
    for (auto [entt1, arm1, pos1] : armies.each()) {
        for (auto [entt2, arm2, pos2] : armies.each()) {
            auto enttpair = std::make_pair(entt1, entt2);
            if (entt1 >= entt2 || collision_free.contains(enttpair)) {
                continue;
            }
            if (glm::length(pos1.value - pos2.value) >
                1.F) { // TODO(shelpam): may use another value here?
                continue;
            }
            // Collision happens between entt1 and entt2
            spdlog::error("LJF 碰撞");
            spdlog::debug("Collision detected: {} with {}",
                          static_cast<int>(entt1), static_cast<int>(entt2));
            collision_free.insert({enttpair, now + 1});
        }
    }
}
entt::entity get_active_camera(entt::registry &reg)
{
    for (auto [entity, cam] : reg.view<Camera>().each()) {
        if (cam.is_active) {
            return entity;
        }
    }
    throw std::runtime_error("couldn't find active camera");
}
glm::mat4 get_active_view_mat(entt::registry &reg)
{
    for (auto [entity, cam, pos] : reg.view<Camera, Position>().each()) {
        if (cam.is_active) {
            return cam.calc_view_matrix(pos.value);
        }
    }
    throw std::runtime_error("couldn't find active camera");
}
void render_system(entt::registry &registry, float now, glm::mat4 const &proj)
{
    auto view_mat = get_active_view_mat(registry);

    auto renderables = registry.view<Renderable, Position>();
    for (auto [entity, renderable, pos] : renderables.each()) {
        auto const *shader = renderable.shader;
        glm::mat4 model(1.);
        model = glm::translate(model, pos.value);
        shader->uniform_mat3("transposed_inverse_model",
                             glm::transpose(glm::inverse(model)));
        shader->uniform_mat4("model", model);
        shader->uniform_mat4("view", view_mat);
        shader->uniform_mat4("projection", proj);
        renderable.mesh->bind_diffuse_and_specular(0, 1);
        shader->uniform_1i("material.diffuse", 0);
        shader->uniform_1i("material.specular", 1);
        shader->uniform_1f("material.shininess", 64);

        auto dlights = registry.view<Light, Directional_light>();
        for (auto [entity, light, dlight] : dlights.each()) {
            shader->uniform_vec3("dlight.light.ambient", light.ambient);
            shader->uniform_vec3("dlight.light.diffuse", light.diffuse);
            shader->uniform_vec3("dlight.light.specular", light.specular);
            shader->uniform_vec3("dlight.dir", dlight.dir);
        }
        auto plights = registry.view<Light, Point_light, Position>();
        for (auto [entity, light, plight, pos] : plights.each()) {
            constexpr float mul = 8;
            shader->uniform_vec3("plight.light.ambient", light.ambient * mul);
            shader->uniform_vec3("plight.light.diffuse", light.diffuse * mul);
            shader->uniform_vec3("plight.light.specular", light.specular * mul);
            shader->uniform_vec3("plight.position", pos.value);
            shader->uniform_1f("plight.constant", plight.constant);
            shader->uniform_1f("plight.linear", plight.linear);
            shader->uniform_1f("plight.quadratic", plight.quadratic);
        }
        auto slights = registry.view<Light, Spot_light, Position>();
        for (auto [entity, light, slight, pos] : slights.each()) {
            shader->uniform_vec3("slight.light.ambient", light.ambient);
            shader->uniform_vec3("slight.light.diffuse", light.diffuse);
            shader->uniform_vec3("slight.light.specular", light.specular);
            shader->uniform_1f("slight.constant", slight.constant);
            shader->uniform_1f("slight.linear", slight.linear);
            shader->uniform_1f("slight.quadratic", slight.quadratic);
            shader->uniform_vec3("slight.position", pos.value);
            shader->uniform_vec3("slight.dir", slight.dir);
            shader->uniform_1f("slight.cut_off", slight.cut_off);
            shader->uniform_1f("slight.outer_cut_off", slight.outer_cut_off);
        }

        auto cam = get_active_camera(registry);
        shader->uniform_vec3("cameraPos", registry.get<Position>(cam).value);
        renderable.mesh->render(*shader);
    }
}
