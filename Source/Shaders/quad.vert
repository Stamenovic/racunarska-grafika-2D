#version 330 core

layout (location = 0) in vec2 aPos;

uniform float uX;
uniform float uY;
uniform float uSX;
uniform float uSY;

uniform mat4 uTransform;

void main()
{

	float px =	aPos.x * uSX + uX;
	float py =	aPos.y * uSY + uY;

	gl_Position =vec4 (px, py, 0.0, 1.0);
}