#version 450 core
#pragma debug(on)

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in vec3 vTangent;
layout (location = 3) in vec3 vBitangent;
layout (location = 4) in vec3 vNormal;
layout (location = 5) in mat2x4 boneIds;
layout (location = 7) in mat2x4 boneWeights;
//layout (location = 5) in mat4 model_matrix;

uniform mat4x4 mat_mvp;
uniform mat4x4 mat_world;
uniform samplerBuffer sb1;
uniform mat4x4 gBones[200];

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
	//vec4 weights = boneWeights;
	//weights.w = 1.0f - dot( weights.xyz, vec3(1, 1, 1));

	/**/
	vec4 objPos = ((gBones[int(boneIds[0].x)]) * localPos) * boneWeights[0].x;
	objPos += ((gBones[int(boneIds[0].y)]) * localPos) * boneWeights[0].y;
	objPos += ((gBones[int(boneIds[0].z)]) * localPos) * boneWeights[0].z;
	objPos += ((gBones[int(boneIds[0].w)]) * localPos) * boneWeights[0].w;

	objPos += ((gBones[int(boneIds[1].x)]) * localPos) * boneWeights[1].x;
	objPos += ((gBones[int(boneIds[1].y)]) * localPos) * boneWeights[1].y;
	objPos += ((gBones[int(boneIds[1].z)]) * localPos) * boneWeights[1].z;
	objPos += ((gBones[int(boneIds[1].w)]) * localPos) * boneWeights[1].w;
	

	/*
	mat4 boneTrans = gBones[(boneIds[0])] * weights[0];
	boneTrans += gBones[(boneIds[1])] * weights[1];
	boneTrans += gBones[(boneIds[2])] * weights[2];
	boneTrans += gBones[(boneIds[3])] * weights[3];
	vec4 objPos = boneTrans * localPos;
	*/

	//----reset pos to original pos
	//objPos = localPos;
	

	gl_Position = mat_mvp * objPos;
	
	worldPos = (mat_world * objPos).xyz;
	worldNormal = normalize(vNormal * inverse(mat3(mat_world)));

	uv = vUV;
	worldMat = mat_world;
	//testIds = vec4((boneIds[1].x) / 255.0, (boneIds[1].y) / 255.0, (boneIds[1].z) / 255.0, boneIds[0].w);
	//testIds = boneWeights[0];
}