set_project("mb")
set_languages("cxxlatest")

includes("**/xmake.lua")

add_rules("mode.debug", "mode.release")
set_rundir("$(projectdir)")

-- dependencies
add_requires("entt", "glfw", "glm", "spdlog", "stb")

target("mountblade-clone")
    set_kind("binary")
    add_files("mb/*.cpp")
    add_deps("glad")
    add_packages("entt", "glfw", "glm", "spdlog", "stb")
    add_includedirs(".")
