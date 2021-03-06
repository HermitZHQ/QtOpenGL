#version 450 core
#pragma debug(on)

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec3 vNormal;
layout (location = 5) in mat4 model_matrix;

uniform mat4x4 mat_mvp;
uniform mat4x4 mat_world;
uniform vec3 worldCamPos;

//----out vars
out Vertex {
	vec2 uv;
	vec3 camPosWorld;
	mat4x4 matWorld;
};

void main()
{
	gl_Position = vec4(vPosition, 1);
	camPosWorld = worldCamPos;

	matWorld = mat_world;
	uv = vUV;
	uv.y = 1.0 - uv.y;
}