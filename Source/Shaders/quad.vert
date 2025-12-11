#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTex;

out vec2 vTex;

uniform float uX;
uniform float uY;
uniform float uSX;
uniform float uSY;

uniform mat4 uTransform;

void main()
{

	vec2 scaled = aPos * vec2(uSX, uSY);
	vec2 translated = scaled + vec2(uX, uY);

	gl_Position =vec4 (translated, 0.0, 1.0);

	vTex = aTex;
}