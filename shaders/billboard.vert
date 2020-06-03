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
uniform mat4x4 projMat;

uniform samplerBuffer sb1;

//----out vars
out Vertex {
	vec2 uv;
	vec3 skyboxUV;
	vec3 worldNormal;
	vec3 worldPos;
	mat4x4 worldMat;
	vec3 camPosWorld;
	mat3x3 tangentToModelMat;
};

void main()
{
	gl_Position = mat_mvp * vec4(vPosition, 1);
	worldPos = (mat_world * vec4(vPosition, 1)).xyz;

	// 
	vec3 camPosInModelSpace = (inverse(mat_world) * vec4(worldCamPos, 1)).xyz;
	vec3 axisZ = normalize(camPosInModelSpace);
	vec3 axisY = axisZ.y > 0.999 ? vec3(0, 0, 1) : vec3(0, 1, 0);
	vec3 axisX = normalize(cross(axisY, axisZ));
	axisY = normalize(cross(axisZ, axisX));

	mat4x4 rotate = mat4x4(vec4(axisX, 0), vec4(axisZ, 0), vec4(-axisY, 0), vec4(0, 0, 0, 1));
	rotate = mat4x4(vec4(axisX, 0), vec4(axisY, 0), vec4(axisZ, 0), vec4(0, 0, 0, 1));

	vec3 vertexModelSpace = (transpose(rotate) * vec4(vPosition.x, vPosition.y, vPosition.z, 1)).xyz;
	//vertexModelSpace = axisX * vPosition.x + axisZ * vPosition.y + -axisY * vPosition.z;

	gl_Position = mat_mvp * vec4(vertexModelSpace, 1);
	worldPos = (mat_world * vec4(vertexModelSpace, 1)).xyz;

	vec3 T = normalize(mat3(mat_world) * vTangent);
	//T = mat3(mat_world) * vTangent;
	vec3 B = normalize(mat3(mat_world) * vBitangent);
	//B = mat3(mat_world) * vBitangent;
	vec3 N = normalize(vNormal * inverse(mat3(mat_world)));
	//N = vNormal * inverse(mat3(mat_world));
	
	// re-orthogonalize T with respect to N
	//T = (T - dot(T, N) * N);

	B = cross(N, T);

	tangentToModelMat[0] = vTangent;
	tangentToModelMat[1] = vBitangent;
	tangentToModelMat[2] = vNormal;

	tangentToModelMat = mat3(mat_world) * tangentToModelMat;

	//tangentToModelMat = mat3(T, B, N);//for method 1

	camPosWorld = worldCamPos;
	vec3 worldView = worldPos - worldCamPos;
	worldNormal = normalize(vNormal * inverse(mat3(mat_world)));
	skyboxUV = reflect(worldView, worldNormal);
	skyboxUV = worldView - 2 * dot(worldNormal, worldView) * worldNormal;

	uv = vUV;
	worldMat = mat_world;
}