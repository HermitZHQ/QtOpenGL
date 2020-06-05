#version 450 core
#pragma debug(on)

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec3 vNormal;

uniform mat4x4 mat_mvp;
uniform mat4x4 mat_world;
uniform vec3 worldCamPos;

//----out vars
out Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	mat3x3 tangentToModelMat;
};

void main()
{
	gl_Position = (mat_mvp * vec4(vPosition, 1));
	
	worldPos = (mat_world * vec4(vPosition, 1)).xyz;
	worldNormal = normalize(vNormal * inverse(mat3(mat_world)));

	skyboxUV = vPosition;
	worldMat = mat_world;
}