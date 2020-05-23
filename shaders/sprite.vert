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
uniform samplerBuffer sb1;

//----out vars
out Vertex {
	vec2 uv;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	mat3x3 tangentToModelMat;
};

void main()
{
	gl_Position = mat_mvp * vec4(vPosition, 1);
	
	worldPos = (mat_world * vec4(vPosition, 1)).xyz;
	worldNormal = normalize(vNormal * inverse(mat3(mat_world)));

	// sprite test
	//gl_PointSize = 5000;

	uv = vUV;
	worldMat = mat_world;
}