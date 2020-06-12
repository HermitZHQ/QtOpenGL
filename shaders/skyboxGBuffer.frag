#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 3) out vec3 gSkybox;

uniform samplerCube skybox;
uniform vec4 ambientColor;
uniform mat4x4 viewMat;

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
	gPosition = (viewMat * vec4(worldPos, 1)).xyz;
	gSkybox = texture(skybox, skyboxUV).rgb;
	//gSkybox = vec3(1, 1, 0);
}