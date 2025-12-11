#version 330 core

in vec2 vTex;
out vec4 FragColor;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform int uUseTexture; //0-boja, 1-tekstura

void main()
{
	if(uUseTexture == 1)
	{
		vec4 texColor = texture(uTexture, vTex);
		FragColor =texColor * uColor;
	}
	else
	{
		FragColor = uColor;
	}
	
}

