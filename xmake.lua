set_project("mb")
set_languages("cxxlatest")

includes("**/xmake.lua")

add_rules("mode.debug", "mode.release")
set_rundir("$(projectdir)")

-- dependencies
add_requires("assimp")
add_requires("entt")
add_requires("freetype")
add_requires("glfw")
add_requires("glm")
add_requires("spdlog")
add_requires("stb")

add_cxxflags("-Wmissing-field-initializers")

target("mountblade-clone")
    set_kind("binary")
    add_files("mb/*.cpp")
    add_headerfiles("mb/*.h")
    add_deps("glad")
    add_packages("assimp", "entt", "freetype", "glfw", "glm", "spdlog", "stb")
    add_includedirs("$(projectdir)")
