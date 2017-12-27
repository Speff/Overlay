#ifndef WINDOW_BOILERPLATE
#define WINDOW_BOILERPLATE

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cimgui.h>
#include <imgui_impl_glfw_gl3.h>


struct point {
	GLfloat x;
	GLfloat y;
} point;

void initGLFW(GLFWwindow**, int*, int*);
void compileShaders(GLuint, GLuint, GLuint);
void renderFBO(GLuint, int, int);
void bindFBO(GLuint, GLuint, int, int);

// GLFW functions =============================================================
void keyCallback(GLFWwindow*, int, int, int, int);
void closeCallback(GLFWwindow*);
void errorCallback(int e, const char *d);
// ============================================================================

#endif