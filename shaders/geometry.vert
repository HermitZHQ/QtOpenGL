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
uniform mat4x4 viewMat;

//----out vars
out VS_OUT{
	vec2 uv;
	vec3 normal;
}vs_out;	

void main()
{
	gl_Position = viewMat * mat_world * vec4(vPosition, 1);
	vs_out.uv = vUV;
	vs_out.normal = normalize((transpose(inverse(viewMat * mat_world)) * vec4(vNormal, 0)).xyz);
	//vs_out.normal = normalize((viewMat * mat_world * vec4(vNormal, 1)).xyz);
}