#version 450 core
#pragma debug(on)

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec3 vNormal;
layout (location = 5) in mat4 model_matrix;
layout (location = 6) in ivec4 boneIds;
layout (location = 7) in vec4 boneWeights;

uniform mat4x4 mat_mvp;
uniform mat4x4 mat_world;
uniform samplerBuffer sb1;

uniform mat4 gBones[100];

//----out vars
out Vertex {
	vec2 uv;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	mat3x3 tangentToModelMat;
	vec4 testIds;
};

void main()
{
	vec4 localPos = vec4(vPosition, 1);
	vec4 weights = boneWeights;
	weights.w = 1.0f - dot( weights.xyz, vec3(1, 1, 1));
	vec4 objPos = ((gBones[boneIds[0]]) * localPos) * weights[0];
	objPos += ((gBones[boneIds[1]]) * localPos) * weights[1];
	objPos += ((gBones[boneIds[2]]) * localPos) * weights[2];
	objPos += ((gBones[boneIds[3]]) * localPos) * weights[3];

	gl_Position = mat_mvp * objPos;
	
	worldPos = (mat_world * objPos).xyz;
	worldNormal = normalize(vNormal * inverse(mat3(mat_world)));

	uv = vUV;
	worldMat = mat_world;
	testIds = boneWeights;
}