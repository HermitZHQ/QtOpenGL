#version 450 core

layout (location = 0) out vec4 fColor;

uniform samplerCube skybox;

//----in vars
in Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	mat3x3 tangentToModelMat;
};

void main()
{
	fColor = texture(skybox, skyboxUV);
	//fColor = vec4(1, 0, 0, 1);
}