#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void cursorpos_callback(GLFWwindow *window, double xpos, double ypos);
void mousebutton_callback(GLFWwindow *window, int button, int action, int mods);
void framebuffersize_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
