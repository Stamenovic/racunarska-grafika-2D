#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
int endProgram(std::string message);
unsigned int createShader(const char* vsSource, const char* fsSource);
unsigned char* resizeImage(unsigned char* data, int w, int h, int channels, int newW, int newH);
unsigned loadImageToTexture(const char* filePath);
GLFWcursor* loadImageToCursor(const char* filePath);