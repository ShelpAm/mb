#include <mb/systems.h>

#include <mb/components.h>
#include <mb/lights.h>
#include <mb/mesh.h>
#include <mb/shader-program.h>

bool chance(float p)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0F, 1.0F);
    return dist(gen) < p;
}

void ai_system(entt::registry &registry, float dt)
{
    auto troops = registry.view<Ai_tag, Army, Position>();
    for (auto [entity, army, pos] : troops.each()) {
        // Add cooldown component if missing
        if (!registry.any_of<Ai_cooldown>(entity)) {
            registry.emplace<Ai_cooldown>(entity,
                                          Ai_cooldown{.timer = 0, .total = 1});
        }

        auto &cd = registry.get<Ai_cooldown>(entity);
        cd.timer -= dt; // decrease by frame delta time
        if (cd.timer > 0.0F) {
            continue; // not ready yet
        }
        cd.timer = cd.total; // reset 1 second cooldown

        // Decide action
        if (!registry.any_of<Pathing>(entity)) {
            if (chance(0.3F)) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dist_x(0.0F, 100.0F);
                std::uniform_real_distribution<float> dist_y(0, 0);
                std::uniform_real_distribution<float> dist_z(0.0F, 100.0F);
                glm::vec3 random_pos{dist_x(gen), dist_y(gen), dist_z(gen)};
                registry.emplace<Pathing>(entity, random_pos);
                spdlog::debug("pathing: {} -> ({}, {}, {})",
                              static_cast<int>(entity), random_pos.x,
                              random_pos.y, random_pos.z);
            }
        }
    }
}
void movement_system(entt::registry &registry, float dt,
                     std::vector<std::vector<float>> const &mountain_height)
{
    // Grants velocity to those who have will to pathing to somewhere.
    constexpr double eps{0.5};
    auto pathings = registry.view<Pathing, Position, Velocity>();
    for (auto [entity, pathing, pos, vel] : pathings.each()) {
        // Pathing to x,z
        if (glm::length(
                glm::vec2{pathing.destination.x, pathing.destination.z} -
                glm::vec2{pos.value.x, pos.value.z}) > eps) {
            vel.dir = glm::normalize(pathing.destination - pos.value);
        }
        else {
            vel.dir = {};
            registry.remove<Pathing>(entity);
        }
    }

    // Moves those have velocity to their direction.
    auto moveables = registry.view<Position, Velocity>().each();
    for (auto [entity, pos, vel] : moveables) {
        if (glm::length(vel.dir) < 1e-5) { // Regarded as still
            continue;
        }
        pos.value += glm::normalize(vel.dir) * vel.speed * dt;

        if (registry.all_of<Army>(entity)) {
            // TODO: THIS IS ONLY FOR TEST
            pos.value.y =
                mountain_height[int(pos.value.z) % mountain_height.size()]
                               [int(pos.value.x) % mountain_height[0].size()] +
                2;
        }

        auto p = pos.value;
        spdlog::debug("entity {} pos={},{},{}",
                      static_cast<std::size_t>(entity), p.x, p.y, p.z);
    }
}
void collision_system(entt::registry &registry, float now)
{
    // Pairs in this map won't collide until `value`.
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
    //  auto armies = registry.view<Army, Position>();
    //  for (auto [entt1, arm1, pos1] : armies.each()) {
    //      for (auto [entt2, arm2, pos2] : armies.each()) {
    //          auto enttpair = std::make_pair(entt1, entt2);
    //          if (entt1 >= entt2 || collision_free.contains(enttpair)) {
    //              continue;
    //          }
    //          if (glm::length(pos1.value - pos2.value) >
    //              1.F) { // TODO(shelpam): may use another value here?
    //              continue;
    //          }
    //          // Collision happens between entt1 and entt2
    //          spdlog::debug("Collision detected: {} with {}",
    //                        static_cast<int>(entt1), static_cast<int>(entt2));
    //          if (entt1 == me_ || entt2 == me) {
    //          }
    //          collision_free.insert({enttpair, now + 1});
    //      }
    //  }
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderable.diffuse_map);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, renderable.specular_map);
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

        auto cam = get_active_camera(registry);
        shader->uniform_vec3("cameraPos", registry.get<Position>(cam).value);
        renderable.mesh->render(*shader);
    }
}
