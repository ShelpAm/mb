#include <mb/systems.h>

#include <mb/components.h>
#include <mb/events.h>
#include <mb/game.h>
#include <mb/get-terrain-height.h>
#include <mb/helpers.h>
#include <mb/lights.h>
#include <mb/mesh.h>
#include <mb/model.h>
#include <mb/shader-program.h>
#include <mb/town.h>

#include <random>
// FIXME: Should be removed: ECS shouldn't depend on particular glfwTime.
#include <GLFW/glfw3.h>

void movement_system(entt::registry &reg, float dt,
                     std::vector<std::vector<float>> const &mountain_height)
{
    // Simulates movement of sun
    auto dlights = reg.view<Directional_light>();
    for (auto [entity, dlight] : dlights.each()) {
        // Simulate sun movement (circular arc in x-y plane)
        float angle = glfwGetTime() * 0.5f; // Adjust speed (0.5 radians/sec)
        dlight.dir = glm::normalize(
            glm::vec3(cos(angle), -sin(angle), sin(angle) * 0.5f));
    }

    // Moves those have velocity to their direction.
    if (mountain_height.empty() || mountain_height[0].empty()) {
        throw std::runtime_error(
            "Invalid mountain height data (size too small)");
    }
    auto moveables = reg.view<Position, Velocity>().each();
    for (auto [entity, pos, vel] : moveables) {
        if (glm::length(vel.dir) < 1e-5) { // Regarded as still
            continue;
        }
        pos.value += glm::normalize(vel.dir) * vel.speed * dt;

        if (reg.all_of<Army>(entity)) {
            pos.value.y =
                get_terrain_height(mountain_height, pos.value.x, pos.value.z);
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
void collision_system(entt::registry &registry, entt::dispatcher &dispatcher,
                      float dt)
{
    // Pairs in this map won't collide remaining time.
    // first entity should be less than second entity.
    static std::map<std::pair<entt::entity, entt::entity>, double>
        collision_free;

    // Removes pairs that can perform collision between them.
    for (auto it = collision_free.begin(); it != collision_free.end();) {
        auto &remaining = it->second;
        remaining -= dt;
        if (remaining <= 0) {
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
    auto collidables = registry.view<Collidable, Position>();
    for (auto [e1, pos1] : collidables.each()) {
        for (auto [e2, pos2] : collidables.each()) {
            auto enttpair = std::make_pair(e1, e2);
            if (e1 >= e2 || collision_free.contains(enttpair)) {
                continue;
            }
            if (glm::length(pos1.value - pos2.value) >
                1.F) { // TODO(shelpam): may use another value here?
                continue;
            }
            // Collision happens between entt1 and entt2
            spdlog::error("LJF 碰撞");
            spdlog::debug("Collision detected: {} with {}",
                          static_cast<int>(e1), static_cast<int>(e2));
            if (registry.all_of<Local_player_tag>(e1) ||
                registry.all_of<Local_player_tag>(e2)) {
                auto self = e1;
                auto other = e2;
                if (registry.all_of<Local_player_tag>(other)) {
                    std::swap(self, other);
                }
                dispatcher.trigger(Collision_event{
                    .registry = &registry, .self{self}, .other{other}});
            }
            collision_free.insert({enttpair, 1});
        }
    }
}

void render_system(entt::registry &registry, glm::mat4 const &proj)
{
    auto view_mat = get_active_view_mat(registry);

    auto renderables = registry.view<Renderable, Position>();
    auto me = get_first_local_player(registry);
    auto const &myperc = registry.get<Army>(me).perception;
    for (auto [e, renderable, pos] : renderables.each()) {
        // Unseenable armies for us (local player)
        if (registry.all_of<Army>(e) &&
            !std::ranges::contains(myperc.viewable_entity, e)) {
            continue;
        }

        if (renderable.model == nullptr) {
            spdlog::error("model is nullptr, probably somewhere wrong in code");
            throw std::runtime_error("check last error");
        }
        auto const *shader = renderable.shader;
        if (shader == nullptr) {
            spdlog::error(
                "shader is nullptr, probably somewhere wrong in code");
            throw std::runtime_error("check last error");
        }

        glm::mat4 model(1.);
        model = glm::translate(model, pos.value);
        if (registry.all_of<Transform>(e)) { // Scale and rotation
            auto const &trans = registry.get<Transform>(e);
            model = glm::scale(model, trans.scale);
            auto rotx = glm::angleAxis(trans.rotation.x, glm::vec3{1, 0, 0});
            auto roty = glm::angleAxis(trans.rotation.y, glm::vec3{0, 1, 0});
            auto rotz = glm::angleAxis(trans.rotation.z, glm::vec3{0, 0, 1});
            model = glm::mat4(rotz * roty * rotx) * model;
        }
        shader->uniform_mat3("transposed_inverse_model",
                             glm::transpose(glm::inverse(model)));
        shader->uniform_mat4("model", model);
        shader->uniform_mat4("view", view_mat);
        shader->uniform_mat4("projection", proj);
        uniform_lights(registry, *shader);
        auto cam = get_active_camera(registry);
        shader->uniform_vec3("cameraPos", registry.get<Position>(cam).value);

        renderable.model->render(*shader);
    }
}

void town_script(entt::registry &reg, float dt)
{
    // Checks Town integrity
    auto towns = reg.view<comp::Town_tag>();
    for (auto [e] : towns.each()) {
        auto *market = reg.try_get<comp::Market>(e);
        if (market == nullptr) {
            spdlog::error("Town_tag doesn't own Market, but it should");
            throw std::logic_error("check last error");
        }

        for (auto item_e : market->items) {
            auto *item = reg.try_get<comp::Item>(item_e);
            if (item == nullptr) {
                spdlog::error("Invalid item id {}", static_cast<int>(item_e));
                throw std::logic_error("check last error");
            }
        }
    }
}

void collision_script(entt::registry &reg, entt::dispatcher &disp)
{
    disp.update<Collision_event>();
}

void camera_script(entt::registry &reg, GLFWwindow *window,
                   View_mode current_view_mode)
{
    { // MANAGE VIEW MODE: Set active camera
        switch (current_view_mode) {
        case View_mode::God:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
            break;
        case View_mode::First_player:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        }
        auto cameras = reg.view<Camera, View_mode>();
        for (auto [entity, cam, view_mode] : cameras.each()) {
            cam.is_active = view_mode == current_view_mode;
        }
    }
}
