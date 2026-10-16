// Emscripten 3.1.6's GLFW port predates these two window-management functions.
// Browsers own resizability and CSS pixel density; the game sizes its canvas explicitly.
// Newer toolchains use their real implementations without this compatibility header.
#pragma once
#include <emscripten/version.h>
#if __EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ < 50
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
static inline void glfwSetWindowAttrib(GLFWwindow* window,int attribute,int value) {
    (void)window; (void)attribute; (void)value;
}
static inline void glfwSetWindowContentScaleCallback(GLFWwindow* window,void (*callback)(GLFWwindow*,float,float)) {
    (void)window; (void)callback;
}
#endif
