#version 450 core

//layout (location = 0) out vec4 color;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec3 fColor;

void main()
{
	//color =  vec4(fColor, 1);
	gAlbedo = fColor;
}